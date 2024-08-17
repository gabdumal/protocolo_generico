#include <network.hpp>
#include <console_colors.hpp>
#include <generic_protocol_constants.hpp>
#include <iostream>

using namespace std;

/* Construction */

Network::Network(string name)
{
    this->name = name;
    networkThread = new thread([this]()
                               {
        while (true)
        {
            if (!messages.empty())
            {
                lock_guard<mutex> lock(messagesMutex);
                Message message = messages.front();
                messages.pop();
                this->processMessage(message);
            }
        } });
    networkThread->detach();
}

Network::~Network()
{
    // Wait for all messages to be processed
    while (!messages.empty())
    {
        chrono::milliseconds timeSpan(1000);
        this_thread::sleep_for(timeSpan);
    }
    delete networkThread;
}

/* Getters */

string Network::getName()
{
    return name;
}

/* Methods */

bool Network::connectEntity(Entity entity)
{
    entities.insert(pair<uuids::uuid, Entity>(entity.getId(), entity));
    return true;
}

void Network::disconnectEntity(Entity entity)
{
    entities.erase(entity.getId());
}

bool Network::receiveMessage(Message message)
{
    // Simulate packet loss
    if (rand() % 100 < PACKET_LOSS_PROBABILITY * 100)
    {
        stringstream outputStream;
        setColor(outputStream, TextColor::YELLOW);
        outputStream << "Message " << "[" << message.getId() << "] " << "has been lost in the network " << this->getName() << "!" << endl;
        resetColor(outputStream);
        cout << outputStream.str();
        return false;
    }

    try
    {
        lock_guard<mutex> lock(messagesMutex);
        messages.push(message);
    }
    catch (const exception &e)
    {
        cerr << "Failed to receive message: " << e.what() << endl;
        return false;
    }
    return true;
}

bool Network::processMessage(Message message)
{
    auto targetEntity = entities.find(message.getTargetEntityId());
    if (targetEntity != entities.end())
    {
        bool hasBeenSent = this->sendMessage(message, targetEntity->second);
        return hasBeenSent;
    }
    else
    {
        stringstream outputStream;
        setColor(outputStream, TextColor::RED);
        outputStream << "Target entity " << "[" << message.getTargetEntityId() << "] " << "is not connected to the network " << this->getName() << "!" << endl;
        resetColor(outputStream);
        cerr << outputStream.str();
        return false;
    }
}

bool Network::sendMessage(Message message, Entity targetEntity)
{
    bool hasBeenReceived = targetEntity.receiveMessage(message);
    return hasBeenReceived;
}
