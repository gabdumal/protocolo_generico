#include <generic_protocol.hpp>
#include <constants.hpp>
#include <iostream>

using namespace std;

// Define the static UUID Generator
uuids::uuid_random_generator *GenericProtocol::uuidGenerator = nullptr;

/* Construction */

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
    Network network = Network("Zircônia", this->uuidGenerator);
    outputStream << TAB << "Network: " << network.getName() << endl
                 << endl;
    cout << outputStream.str();
    outputStream.str("");

    outputStream << "Creating entities" << endl;
    Entity entityA = GenericProtocol::createEntity("Aroeira",
                                                   [&outputStream](string message)
                                                   {
                                                       outputStream << TAB << message << endl;
                                                   });
    outputStream << endl;
    Entity entityB = GenericProtocol::createEntity("Baobá", [&outputStream](string message)
                                                   { outputStream << TAB << message << endl; });
    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");

    outputStream << "Connecting entities to the network " << network.getName() << endl;
    network.connectEntity(entityA);
    outputStream << TAB << "Entity " << entityA.getName() << " [" << entityA.getId() << "] connected to network" << endl;
    network.connectEntity(entityB);
    outputStream << TAB << "Entity " << entityB.getName() << " [" << entityB.getId() << "] connected to network" << endl;
    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");

    sendMessage(entityA, entityB, "Hello, Baobá!", Code::SYN, network, outputStream);

    chrono::milliseconds timeSpan(rand() % 30000);
    this_thread::sleep_for(timeSpan);

    outputStream << "Entities' storage" << endl;
    outputStream << TAB << entityA.getName() << " [" << entityA.getId() << "]" << endl;
    entityA.printStorage([&outputStream](string message)
                         { outputStream << TAB << message << endl; });
    outputStream << endl;
    outputStream << TAB << entityB.getName() << " [" << entityB.getId() << "]" << endl;
    entityB.printStorage([&outputStream](string message)
                         { outputStream << TAB << message << endl; });
    cout << outputStream.str();
}

/* Static methods */

Entity GenericProtocol::createEntity(string name, function<void(string)> printMessage)
{
    Entity entity = Entity(uuidGenerator, name);
    printMessage(entity.getName() + " [" + to_string(entity.getId()) + "]");
    entity.printStorage({[&printMessage](string message)
                         {
                             printMessage(TAB + message);
                         }});
    return entity;
}

void GenericProtocol::sendMessage(Entity &source, Entity &target, string messageContent, Code messageCode, Network &network, ostringstream &outputStream)
{
    outputStream << "Sending message" << endl;

    Message message = Message(uuidGenerator, source.getId(), target.getId(), messageContent, messageCode);

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
