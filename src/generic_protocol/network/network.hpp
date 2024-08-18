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

using namespace std;

class Network
{
private:
    string name;
    unordered_map<uuids::uuid, Entity> entities;
    queue<Message> messages;
    mutex messagesMutex;
    thread *networkThread;
    list<future<bool>> messagesFutures;
    mutex futuresMutex;

    bool processMessage(Message &message);
    bool sendMessage(Message message, Entity targetEntity);

public:
    /* Construction */
    Network(string name);
    ~Network();

    /* Getters */
    string getName();

    /* Methods */
    bool connectEntity(Entity entity);
    void disconnectEntity(Entity entity);
    bool receiveMessage(Message message);
};

#endif // _NETWORK_HPP
