#include <generic_protocol.hpp>
#include <iostream>

using namespace std;

// Define the static UUID Generator
uuids::uuid_random_generator *GenericProtocol::uuidGenerator = nullptr;

/* Construction*/

GenericProtocol::GenericProtocol(uuids::uuid_random_generator *uuidGenerator)
{
    this->uuidGenerator = uuidGenerator;
}

GenericProtocol::~GenericProtocol()
{
}

/* Methods */

void GenericProtocol::run()
{
    cout << "01. Generic protocol" << endl
         << endl;

    Entity entityA = GenericProtocol::createEntity("Aroeira");
    Entity entityB = GenericProtocol::createEntity("Baoba");
    cout << endl;

    uuids::uuid const messageId = (*uuidGenerator)();
    string content = "Banana?\nMaça?";
    Message message = Message(messageId, content);
    sendMessage(entityA, message);
    cout << endl;
}

/* Static methods */

Entity GenericProtocol::createEntity(string name)
{
    cout << "Creating entity " << name << endl;
    Entity entity = Entity(name);
    return entity;
}

void GenericProtocol::sendMessage(Entity entity, Message message)
{
    cout << "Sending message" << endl;
    cout << "Origin: " << entity.getName() << endl;
    entity.sendMessage(message);
    cout << endl;
}