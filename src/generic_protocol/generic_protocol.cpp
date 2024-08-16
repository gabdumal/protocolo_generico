#include <generic_protocol.hpp>
#include <iostream>
#include "constants.hpp"

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

    sendMessage(entityA, entityB, "Hello, Baoba!");
    cout << endl;
}

/* Static methods */

Entity GenericProtocol::createEntity(string name)
{
    cout << "Creating entity " << name << endl;
    Entity entity = Entity(uuidGenerator, name);
    return entity;
}

void GenericProtocol::sendMessage(Entity source, Entity target, string messageContent)
{
    cout << "Creating message " << endl;
    Message message = Message(uuidGenerator, source.getId(), target.getId(), messageContent);

    cout << "Sending message" << " [" << message.getId() << "]" << endl;

    cout << TAB << "Source entity: " << source.getName() << " [" << source.getId() << "]" << endl;

    cout << TAB << "Target entity: " << target.getName() << " [" << target.getId() << "]" << endl;

    cout << TAB << "Message:" << endl;
    message.print(
        [](string message)
        {
            cout << TAB << TAB << message << endl;
        });

    source.sendMessage(message);

    cout << endl;
}