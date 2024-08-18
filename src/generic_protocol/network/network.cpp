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
    // Wait for all threads to finish
    while (true)
    {
        std::lock_guard<std::mutex> lock(futuresMutex);
        if (messagesFutures.empty())
            break;

        // Wait for the first future in the list
        messagesFutures.front().wait();

        // Remove the processed future from the list
        messagesFutures.pop_front();
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
        ostringstream outputStream;
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

bool Network::processMessage(Message &message)
{
    promise<bool> messagePromise;
    auto messageFuture = messagePromise.get_future();

    lock_guard<mutex> lock(futuresMutex);
    messagesFutures.push_back(move(messageFuture));

    thread([this, message = move(message), promise = move(messagePromise)]() mutable
           {
               // Simulate network latency
               chrono::milliseconds timeSpan(rand() % NETWORK_LATENCY);
               this_thread::sleep_for(timeSpan);

               auto targetEntity = entities.find(message.getTargetEntityId());
               if (targetEntity != entities.end())
               {
                   bool hasBeenSent = this->sendMessage(message, targetEntity->second);
                   return hasBeenSent;
               }
               else
               {
                   ostringstream outputStream;
                   setColor(outputStream, TextColor::RED);
                   outputStream << "Target entity " << "[" << message.getTargetEntityId() << "] " << "is not connected to the network " << this->getName() << "!" << endl;
                   resetColor(outputStream);
                   cerr << outputStream.str();
                   return false;
               }
               
               std::lock_guard<std::mutex> lock(futuresMutex);
               messagesFutures.remove_if([](const std::future<bool> &fut)
                                         { return fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }); })
        .detach();
    return true;
}

bool Network::sendMessage(Message message, Entity targetEntity)
{
    bool hasBeenReceived = targetEntity.receiveMessage(message);
    return hasBeenReceived;
}
