#include "generic_protocol.hpp"
#include "../util/util.hpp"
#include <iostream>
#include <queue>

using namespace std;

/* Static members */
shared_ptr<uuids::uuid_random_generator> GenericProtocol::uuidGenerator;

/* Auxiliary */

unique_ptr<Network> createNetwork(ostringstream &outputStream, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    outputStream << "Creating network" << endl;
    cout << outputStream.str();
    outputStream.str("");
    unique_ptr<Network> network = Network::createNetwork("Zircônia", uuidGenerator);
    outputStream << TAB << "Network: " << network->getName() << endl
                 << endl;
    cout << outputStream.str();
    outputStream.str("");
    return network;
}

pair<shared_ptr<Entity>, shared_ptr<Entity>> createEntities(ostringstream &outputStream)
{
    outputStream << "Creating entities" << endl;
    auto printMessage = [&outputStream](string message)
    {
        outputStream << TAB << message << endl;
    };
    shared_ptr<Entity> entityA = GenericProtocol::createEntity("Aroeira", printMessage);
    outputStream << endl;
    shared_ptr<Entity> entityB = GenericProtocol::createEntity("Baobá", printMessage);
    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");
    return make_pair(entityA, entityB);
}

void connectEntitiesToNetwork(Network &network, shared_ptr<Entity> entityA, shared_ptr<Entity> entityB, ostringstream &outputStream)
{
    outputStream << "Connecting entities to the network " << network.getName() << endl;
    network.connectEntity(entityA);
    outputStream << TAB << "Entity " << entityA->getName() << " [" << entityA->getId() << "] connected to network" << endl;
    network.connectEntity(entityB);
    outputStream << TAB << "Entity " << entityB->getName() << " [" << entityB->getId() << "] connected to network" << endl;
    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");
}

void printEntitiesStorage(shared_ptr<Entity> entityA, shared_ptr<Entity> entityB, ostringstream &outputStream)
{
    outputStream << "Entities' storage" << endl;
    outputStream << TAB << entityA->getName() << " [" << entityA->getId() << "]" << endl;
    entityA->printStorage([&outputStream](string message)
                          { outputStream << TAB << message << endl; });
    outputStream << endl;
    outputStream << TAB << entityB->getName() << " [" << entityB->getId() << "]" << endl;
    entityB->printStorage([&outputStream](string message)
                          { outputStream << TAB << message << endl; });
    cout << outputStream.str();
}

/* Construction */

GenericProtocol::GenericProtocol(shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    GenericProtocol::uuidGenerator = uuidGenerator;
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

    unique_ptr<Network> network = createNetwork(outputStream, uuidGenerator);
    auto [entityA, entityB] = createEntities(outputStream);
    connectEntitiesToNetwork(*network, entityA, entityB, outputStream);

    // Establish connection between entities
    GenericProtocol::sendMessage(entityA, entityB, "SYN", Code::SYN, *network, outputStream);

    deque<string> contents = {
        // "Hello, Baobá!",
        // "Fragment 1",
        // "Fragment 2",
        // "Fragment 3",
        // "Fragment 4",
        // "Fragment 5",
        "Fragment 6"};

    // for (string content : contents)
    // {
    //     GenericProtocol::sendMessage(entityA, entityB, content, Code::DATA, *network, outputStream);
    // }

    printEntitiesStorage(entityA, entityB, outputStream);
}

/* Static methods */

shared_ptr<Entity> GenericProtocol::createEntity(string name, function<void(string)> printMessage)
{
    shared_ptr<Entity> entity = make_unique<Entity>(name, uuidGenerator);
    printMessage(entity->getName() + " [" + to_string(entity->getId()) + "]");
    entity->printStorage({[&printMessage](string message)
                          {
                              printMessage(TAB + message);
                          }});
    return entity;
}

void printSendingMessageHeader(shared_ptr<Entity> source, shared_ptr<Entity> target, string messageContent, Code messageCode, Network &network, ostringstream &outputStream)
{
    outputStream << "Sending message" << endl;

    outputStream << TAB << "Source entity: " << source->getName() << " [" << source->getId() << "]" << endl;

    outputStream << TAB << "Target entity: " << target->getName() << " [" << target->getId() << "]" << endl;

    outputStream << TAB << "Message content: " << messageContent << endl;

    outputStream << TAB << "Message code: " << messageCode << endl;

    outputStream << TAB << "Network: " << network.getName() << endl;
}

void printSendingMessageFooter(bool hasBeenProcessed, ostringstream &outputStream)
{
    outputStream << endl;
    cout << outputStream.str();
    outputStream.str("");
}

void GenericProtocol::sendMessage(shared_ptr<Entity> source, shared_ptr<Entity> target, string messageContent, Code messageCode, Network &network, ostringstream &outputStream)
{
    Message message = Message(uuidGenerator, source->getId(), target->getId(), messageContent, messageCode);

    printSendingMessageHeader(source, target, messageContent, messageCode, network, outputStream);

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
