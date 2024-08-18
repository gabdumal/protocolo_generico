#include <network.hpp>
#include <console_colors.hpp>
#include <generic_protocol_constants.hpp>
#include <iostream>

using namespace std;

/* Construction */

Network::Network(string name)
{
    this->name = name;
    this->processingMessagesCount = 0;
    this->stopThread = false;

    this->networkThread = thread([this]()
                                 {
       while (true) {
            unique_lock<mutex> lock(this->messagesMutex);
             if (stopThread && messages.empty()) {
                break;
            }
            if (!this->messages.empty()) {
                Message message = this->messages.front();
                this->messages.pop();
                lock.unlock();
                this->processMessage(message);
            } else {
                this->messageProcessedCV.wait(lock); // Wait for new messages
            }
        } });
}

Network::~Network()
{
    {
        lock_guard<mutex> lock(this->messagesMutex);
        this->stopThread = true;
        this->messageProcessedCV.notify_all();
    }
    if (this->networkThread.joinable())
    {
        this->networkThread.join();
    }
}

/* Getters */

string Network::getName() const
{
    return name;
}

/* Methods */

bool Network::connectEntity(Entity &entity)
{
    lock_guard<mutex> lock(this->entitiesMutex);
    this->entities[entity.getId()] = &entity;
    this->messageProcessedCV.notify_all();
    return true;
}

void Network::disconnectEntity(uuids ::uuid entityId)
{
    lock_guard<mutex> lock(this->entitiesMutex);
    this->entities.erase(entityId);
    this->messageProcessedCV.notify_all();
}

bool Network::receiveMessage(Message message)
{
    // Simulate packet loss
    if (rand() % 100 < PACKET_LOSS_PROBABILITY * 100)
    {
        ostringstream outputStream;
        setColor(outputStream, TextColor::YELLOW);
        outputStream << "Message " << "[" << message.getId() << "] " << "has been lost in the network " << this->getName() << "!" << endl;
        resetColor(outputStream);
        cout << outputStream.str();
        return false;
    }

    try
    {
        lock_guard<mutex> lock(this->messagesMutex);
        this->messages.push(message);
        this->processingMessagesCount++;
        cout << "Message received: " << message.getContent() << endl; // Debugging output
        this->messageProcessedCV.notify_one();                        // Notify the network thread
    }
    catch (const exception &e)
    {
        cerr << "Failed to receive message: " << e.what() << endl;
        return false;
    }
    return true;
}

void Network::processMessage(Message message)
{
    // Simulate network latency
    chrono::milliseconds timeSpan(rand() % NETWORK_LATENCY);
    this_thread::sleep_for(timeSpan);

    // Simulate message corruption
    if (rand() % 100 < CORRUPTION_PROBABILITY * 100)
        message.setCorrupted(true);

    cout << "Processing message: " << message.getContent() << endl; // Debugging output
    this->sendMessage(message);

    // Brackets are used to limit the scope of the lock
    {
        lock_guard<mutex> lock(this->messagesMutex);
        this->processingMessagesCount--;
        if (processingMessagesCount == 0)
        {
            this->messageProcessedCV.notify_all(); // Notify the destructor
        }
    }
}

void Network::sendMessage(Message message)
{
    lock_guard<mutex> lock(this->entitiesMutex);
    auto targetEntityPair = this->entities.find(message.getTargetEntityId());
    if (targetEntityPair != entities.end())
    {
        Entity *entity = targetEntityPair->second;
        if (entity)
        {
            entity->receiveMessage(message);
        }
        else
        {
            this->disconnectEntity(message.getTargetEntityId());
        }
    }
    else
    {
        cerr << "Target entity [" << message.getTargetEntityId() << "] is not connected to the network " << this->getName() << "!" << endl;
    }
}
