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
    cout << "Adding entity " << entity.getName() << " [" << entity.getId() << "]" << endl;
    entities.insert(pair<uuids::uuid, Entity>(entity.getId(), entity));
    return true;
}

void Network::disconnectEntity(Entity entity)
{
    cout << "Removing entity " << entity.getName() << " [" << entity.getId() << "]" << endl;
    entities.erase(entity.getId());
}

bool Network::sendMessage(Message message)
{
    return false;
}