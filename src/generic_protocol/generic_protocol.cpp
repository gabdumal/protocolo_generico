#include <generic_protocol.hpp>
#include <iostream>
#include <constants.hpp>

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
    ostringstream outputStream;

    outputStream << "01. GENERIC PROTOCOL" << endl
                 << endl;

    outputStream << "Creating network" << endl;
    cout << outputStream.str();
    outputStream.str("");
    Network network = Network("Zircônia");
    outputStream << TAB << "Network: " << network.getName() << endl
                 << endl;
    cout << outputStream.str();
    outputStream.str("");

    outputStream << "Creating entities" << endl;
    outputStream << TAB;
    Entity entityA = GenericProtocol::createEntity("Aroeira", outputStream);
    outputStream << TAB;
    Entity entityB = GenericProtocol::createEntity("Baobá", outputStream);
    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");

    outputStream << "Connecting entities to the network " << network.getName() << endl;
    bool hasBeenConnected = network.connectEntity(entityA);
    outputStream << TAB << "Entity " << entityA.getName() << " [" << entityA.getId() << "] connected to network: " << (hasBeenConnected ? "TRUE" : "FALSE") << endl;
    hasBeenConnected = network.connectEntity(entityB);
    outputStream << TAB << "Entity " << entityB.getName() << " [" << entityB.getId() << "] connected to network: " << (hasBeenConnected ? "TRUE" : "FALSE") << endl;
    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");

    for (size_t i = 0; i < 32; i++)
    {
        sendMessage(entityA, entityB, "Hello, Baobá! (" + to_string(i) + ")", network, outputStream);
    }
}

/* Static methods */

Entity GenericProtocol::createEntity(string name, ostringstream &outputStream)
{
    outputStream << "Creating entity " << name << endl;
    Entity entity = Entity(uuidGenerator, name);
    return entity;
}

void GenericProtocol::sendMessage(Entity &source, Entity &target, string messageContent, Network &network, ostringstream &outputStream)
{
    outputStream << "Sending message" << endl;

    Message message = Message(uuidGenerator, source.getId(), target.getId(), messageContent);

    outputStream << TAB << "Source entity: " << source.getName() << " [" << source.getId() << "]" << endl;

    outputStream << TAB << "Target entity: " << target.getName() << " [" << target.getId() << "]" << endl;

    outputStream << TAB << "Message:" << endl;
    message.print(
        [&outputStream](string message)
        {
            outputStream << TAB << TAB << message << endl;
        });

    outputStream << TAB << "Network: " << network.getName() << endl;
    bool hasBeenProcessed = network.receiveMessage(message);
    if (hasBeenProcessed)
        outputStream << TAB << "Message sent through network!" << endl;
    else
        outputStream << TAB << "Message not sent through network!" << endl;

    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");
    cout.flush();
}
