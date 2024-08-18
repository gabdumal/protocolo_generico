#include <network.hpp>
#include <console_colors.hpp>
#include <generic_protocol_constants.hpp>
#include <iostream>

using namespace std;

/* Construction */

Network::Network(string name, uuids::uuid_random_generator *uuidGenerator)
{
    this->uuidGenerator = uuidGenerator;
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

void Network::connectEntity(Entity &entity)
{
    lock_guard<mutex> lock(this->entitiesMutex);
    this->entities[entity.getId()] = &entity;
    this->messageProcessedCV.notify_all();
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
    if (rand() % 100 < GenericProtocolConstants::packetLossProbability * 100)
    {
        if (GenericProtocolConstants::debugInformation)
        {
            this->printInformation("Message " + to_string(message.getId()) + " has been lost in the network " + this->getName() + "!", cout, ConsoleColors::Color::YELLOW);
        }
        return false;
    }

    try
    {
        lock_guard<mutex> lock(this->messagesMutex);
        this->messages.push(message);
        this->processingMessagesCount++;
        this->messageProcessedCV.notify_one(); // Notify the network thread
    }
    catch (const exception &e)
    {
        this->printInformation("Error while receiving message in the network " + this->getName() + ": " + e.what(), cerr, ConsoleColors::Color::RED);
        return false;
    }
    return true;
}

void Network::processMessage(Message message)
{
    // Simulate network latency
    chrono::milliseconds timeSpan(rand() % GenericProtocolConstants::networkLatency);
    this_thread::sleep_for(timeSpan);

    // Simulate message corruption
    if (rand() % 100 < GenericProtocolConstants::packetCorruptionProbability * 100)
    {
        this->printInformation("Message " + to_string(message.getId()) + " has been corrupted in the network " + this->getName() + "!", cout, ConsoleColors::Color::YELLOW);
        message.setCorrupted(true);
    }

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
            bool canSendMessage = entity->sendMessage(message);
            if (!canSendMessage)
            {
                if (GenericProtocolConstants::debugInformation)
                {
                    this->printInformation("Target entity " + entity->getName() + " [" + to_string(entity->getId()) + "] cannot receive the message " + "[" + to_string(message.getId()) + "]!", cout, ConsoleColors::Color::RED);
                }
            }
            else
            {
                if (GenericProtocolConstants::debugInformation)
                {
                    this->printInformation("Message " + to_string(message.getId()) + " has been sent to the target entity " + entity->getName() + " [" + to_string(entity->getId()) + "]!", cout, ConsoleColors::Color::GREEN);
                }
                optional<Message> returnedMessage = entity->receiveMessage(message, this->uuidGenerator);
                if (returnedMessage)
                {
                    this->receiveMessage(returnedMessage.value());
                }
            }
        }
        else
        {
            this->disconnectEntity(message.getTargetEntityId());
        }
    }
    else
    {
        this->printInformation("Target entity [" + to_string(message.getTargetEntityId()) + "] is not connected to the network " + this->getName() + "!", cerr, ConsoleColors::Color::RED);
    }
}

void Network::printInformation(string information, ostream &outputStream, ConsoleColors::Color color) const
{
    string header = "Network " + this->getName();
    ConsoleColors::printInformation(header, information, outputStream, ConsoleColors::Color::BRIGHT_WHITE, ConsoleColors::Color::BLUE, color);
}