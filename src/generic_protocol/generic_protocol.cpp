#include "entity.hpp"
#include "generic_protocol.hpp"
#include <iostream>

using namespace std;

void GenericProtocol::run()
{
    cout << "01. Generic protocol" << endl
         << endl;

    Entity entityA = GenericProtocol::createEntity("Aroeira");
    Entity entityB = GenericProtocol::createEntity("Baoba");
    cout << endl;

    sendMessage(entityA, "Banana?\nMaça?");
    cout << endl;
}

Entity GenericProtocol::createEntity(string name)
{
    cout << "Creating entity " << name << endl;
    Entity entity = Entity(name);
    return entity;
}

void GenericProtocol::sendMessage(Entity entity, string message)
{
    cout << "Sending message" << endl;
    cout << "Origin: " << entity.getName() << endl;
    cout << "=== BEGIN ===" << endl;
    entity.sendMessage(message);
    cout << "=== END ===" << endl
         << endl;
}