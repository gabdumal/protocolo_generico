#include <network.hpp>
#include <console_colors.hpp>
#include <generic_protocol_constants.hpp>
#include <iostream>

using namespace std;

/* Construction */

Network::Network(string name, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    this->uuidGenerator = uuidGenerator;
    this->name = name;
    this->processingMessagesCount = 0;
    this->canStopThread = false;
    this->networkThread = thread([this]()
                                 {
                                     while (true)
                                     {
                                         unique_lock<mutex> lock(this->messagesMutex);
                                         if (this->canStopThread && messages.empty())
                                         {
                                             break;
                                         }
                                         if (!this->messages.empty())
                                         {
                                             Message message = this->messages.front();
                                             this->messages.pop();
                                             lock.unlock();
                                             this->processMessage(message);
                                             lock.lock();
                                         }
                                         else
                                         {
                                             this->messageProcessedCV.wait(lock);
                                         }
                                     } });
}

Network::~Network()
{
    {
        lock_guard<mutex> lock(this->messagesMutex);
        this->canStopThread = true;
        this->messageProcessedCV.notify_all();
    }
    this->joinThread();
    if (GenericProtocolConstants::debugInformation)
    {
        this->printInformation("Network " + this->getName() + " has been destroyed!", cout, ConsoleColors::Color::YELLOW);
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
    this->entities[entity.getId()] = make_shared<Entity>(entity);
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
        this->printInformation("Message [" + to_string(message.getId()) + "] has been corrupted in the network " + this->getName() + "!", cout, ConsoleColors::Color::YELLOW);
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
        shared_ptr<Entity> targetEntity = targetEntityPair->second;
        if (targetEntity)
        {
            auto sourceEntityPair = this->entities.find(message.getSourceEntityId());
            if (sourceEntityPair != entities.end())
            {
                shared_ptr<Entity> sourceEntity = sourceEntityPair->second;
                if (sourceEntity)
                {
                    bool canSendMessage = sourceEntity->sendMessage(message);
                    if (!canSendMessage)
                    {
                        if (GenericProtocolConstants::debugInformation)
                        {
                            this->printInformation("Source entity " + sourceEntity->getName() + " [" + to_string(sourceEntity->getId()) + "] cannot send the message " + "[" + to_string(message.getId()) + "]!", cout, ConsoleColors::Color::RED);
                        }
                    }
                    else
                    {
                        if (GenericProtocolConstants::debugInformation)
                        {
                            this->printInformation("Message " + to_string(message.getId()) + " has been sent to the target entity " + targetEntity->getName() + " [" + to_string(targetEntity->getId()) + "]!", cout, ConsoleColors::Color::GREEN);
                        }
                        optional<Message> returnedMessage = targetEntity->receiveMessage(message, this->uuidGenerator);
                        if (returnedMessage)
                        {
                            this->receiveMessage(returnedMessage.value());
                        }
                    }
                }
                else
                {
                    this->printInformation("Source entity [" + to_string(message.getSourceEntityId()) + "] has been disconnected from the network " + this->getName() + "!", cerr, ConsoleColors::Color::RED);
                    this->disconnectEntity(message.getSourceEntityId());
                }
            }
            else
            {
                this->printInformation("Source entity [" + to_string(message.getSourceEntityId()) + "] is not connected to the network " + this->getName() + "!", cerr, ConsoleColors::Color::RED);
            }
        }
        else
        {
            this->printInformation("Target entity [" + to_string(message.getTargetEntityId()) + "] has been disconnected from the network " + this->getName() + "!", cerr, ConsoleColors::Color::RED);
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
    ConsoleColors::printInformation(header, information, outputStream, ConsoleColors::Color::WHITE, ConsoleColors::Color::BLUE, color);
}

void Network::joinThread()
{
    if (this->networkThread.joinable())
    {
        this->networkThread.join();
    }
}

/* Static Methods */

unique_ptr<Network> Network::createNetwork(string name, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    return unique_ptr<Network>(new Network(name, uuidGenerator));
}
