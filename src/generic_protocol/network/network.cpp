#include <network.hpp>
#include <iostream>

using namespace std;

/* Construction */

Network::Network(string name)
{
    this->name = name;
}

Network::~Network() {}

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

bool Network::sendMessage(Message message)
{
    auto targetEntity = entities.find(message.getTargetEntityId());
    if (targetEntity != entities.end())
    {
        targetEntity->second.receiveMessage(message);
        return true;
    }
    else
    {
        cout << "ERROR: Target entity is not connected to the network " << this->getName() << "!" << endl;
        return false;
    }
}