#ifndef _NETWORK_HPP
#define _NETWORK_HPP

#include <uuid.h>
#include <entity.hpp>
#include <message.hpp>
#include <string>
#include <unordered_map>
#include <queue>
#include <list>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <memory>

using namespace std;

class Network
{
private:
    uuids::uuid_random_generator *uuidGenerator;
    string name;
    thread networkThread;

    unordered_map<uuids::uuid, shared_ptr<Entity>>
        entities;
    mutex entitiesMutex;

    queue<Message> messages;
    mutex messagesMutex;

    condition_variable messageProcessedCV;
    int processingMessagesCount;
    bool canStopThread;

    /* Construction */
    Network(string name, uuids::uuid_random_generator *uuidGenerator);

    /* Methods */
    void processMessage(Message message);
    void sendMessage(Message message);
    void printInformation(string information, ostream &outputStream, ConsoleColors::Color color = ConsoleColors::Color::DEFAULT) const;
    void joinThread();

public:
    /* Destruction */
    ~Network();

    /* Getters */
    string getName() const;

    /* Methods */
    void connectEntity(Entity &entity);
    void disconnectEntity(uuids::uuid entityId);
    bool receiveMessage(Message message);

    /* Static Methods */
    static unique_ptr<Network> createNetwork(string name, uuids::uuid_random_generator *uuidGenerator);
};

#endif // _NETWORK_HPP
