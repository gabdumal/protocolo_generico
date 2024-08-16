#ifndef _NETWORK_HPP
#define _NETWORK_HPP

#include <uuid.h>
#include <entity.hpp>
#include <message.hpp>
#include <string>
#include <map>

using namespace std;

class Network
{
private:
    string name;
    map<uuids::uuid, Entity> entities;

public:
    /* Construction */
    Network(string name);
    ~Network();

    /* Getters */
    string getName();

    /* Methods */
    bool connectEntity(Entity entity);
    void disconnectEntity(Entity entity);
    bool sendMessage(Message message);
};

#endif // _NETWORK_HPP