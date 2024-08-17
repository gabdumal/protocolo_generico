#include <network.hpp>
#include <console_colors.hpp>
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
        outputStream << "Target entity " << "[" << message.getTargetEntityId() << "] " << "is not connected to the network " << this->getName() << "!" << endl;

        setColor(TextColor::RED);
        cerr << outputStream.str();
        resetColor();
        return false;
    }
}

bool Network::sendMessage(Message message, Entity targetEntity)
{
    bool hasBeenReceived = targetEntity.receiveMessage(message);
    return hasBeenReceived;
}