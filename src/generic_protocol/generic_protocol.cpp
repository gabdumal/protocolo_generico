#include "generic_protocol.hpp"

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <pretty_console.hpp>
#include <sstream>
#include <utility>

#include "connection.hpp"
#include "entity.hpp"
#include "message.hpp"

using namespace std;

/* Static members */
shared_ptr<uuids::uuid_random_generator> GenericProtocol::uuid_generator;

/* Auxiliary */

unique_ptr<Network> createNetwork(
    ostringstream &output_stream,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    output_stream << "Creating network" << endl;
    cout << output_stream.str();
    output_stream.str("");
    unique_ptr<Network> network =
        Network::createNetwork("Zircônia", uuid_generator);
    output_stream << PrettyConsole::tab << "Network: " << network->getName()
                  << endl
                  << endl;
    cout << output_stream.str();
    output_stream.str("");
    return network;
}

pair<shared_ptr<Entity>, shared_ptr<Entity>> createEntities(
    list<shared_ptr<Entity>> &entities,
    map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
    ostringstream &output_stream) {
    output_stream << "Creating entities" << endl;
    auto print_message = [&output_stream](string message) {
        output_stream << PrettyConsole::tab << message << endl;
    };

    shared_ptr<Entity> entity_a = GenericProtocol::createEntity(
        "Aroeira", entities, connections, print_message);
    output_stream << endl;
    shared_ptr<Entity> entity_b = GenericProtocol::createEntity(
        "Baobá", entities, connections, print_message);

    output_stream << endl;
    cout << output_stream.str();
    output_stream.str("");
    return make_pair(entity_a, entity_b);
}

void connectEntitiesToNetwork(Network &network, shared_ptr<Entity> entity_a,
                              shared_ptr<Entity> entity_b,
                              ostringstream &output_stream) {
    output_stream << "Connecting entities to the network " << network.getName()
                  << endl;
    network.connectEntity(entity_a);
    output_stream << PrettyConsole::tab << "Entity " << entity_a->getName()
                  << " [" << entity_a->getId() << "] connected to network"
                  << endl;
    network.connectEntity(entity_b);
    output_stream << PrettyConsole::tab << "Entity " << entity_b->getName()
                  << " [" << entity_b->getId() << "] connected to network"
                  << endl;
    output_stream << endl;
    cout << output_stream.str();
    output_stream.str("");
}

void printEntitiesStorage(shared_ptr<Entity> entity_a,
                          shared_ptr<Entity> entity_b,
                          ostringstream &output_stream) {
    output_stream << "Entities' storage" << endl;
    output_stream << PrettyConsole::tab << entity_a->getName() << " ["
                  << entity_a->getId() << "]" << endl;
    entity_a->printStorage([&output_stream](string message) {
        output_stream << PrettyConsole::tab << message << endl;
    });
    output_stream << endl;
    output_stream << PrettyConsole::tab << entity_b->getName() << " ["
                  << entity_b->getId() << "]" << endl;
    entity_b->printStorage([&output_stream](string message) {
        output_stream << PrettyConsole::tab << message << endl;
    });
    cout << output_stream.str();
}

/* Construction */

GenericProtocol::GenericProtocol(
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    GenericProtocol::uuid_generator = uuid_generator;
}

GenericProtocol::~GenericProtocol() {}

/* Methods */

void GenericProtocol::run() {
    ostringstream output_stream;

    output_stream << "01. GENERIC PROTOCOL" << endl << endl;

    unique_ptr<Network> network = createNetwork(output_stream, uuid_generator);

    map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections;

    list<shared_ptr<Entity>> entities;
    auto [entity_a, entity_b] =
        createEntities(entities, connections, output_stream);
    connectEntitiesToNetwork(*network, entity_a, entity_b, output_stream);

    // Establish connection between entities
    GenericProtocol::sendMessage(entity_a, entity_b, "SYN", Message::Code::SYN,
                                 *network, output_stream);

    deque<string> contents = {// "Hello, Baobá!",
                              // "Fragment 1",
                              // "Fragment 2",
                              // "Fragment 3",
                              // "Fragment 4",
                              // "Fragment 5",
                              "Fragment 6"};

    // for (string content : contents) {
    //     GenericProtocol::sendMessage(entity_a, entity_b, content,
    //                                  Message::Code::DATA, *network,
    //                                  output_stream);
    // }

    printEntitiesStorage(entity_a, entity_b, output_stream);
    network->joinThreads();
    printEntitiesStorage(entity_a, entity_b, output_stream);
}

/* Static methods */

// TODO: Refactor this method to use the Entity constructor
shared_ptr<Entity> GenericProtocol::createEntity(
    string name, list<shared_ptr<Entity>> &entities,
    map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
    function<void(string)> print_message) {
    make_shared<uuids::uuid>(GenericProtocol::uuid_generator->operator()());

    Entity entity(
        GenericProtocol::uuid_generator->operator()(), name,
        [&connections](uuids::uuid source_entity_id,
                       uuids::uuid target_entity_id, uuids::uuid message_id,
                       ConnectionStep step) {
            Connection::connect(connections, source_entity_id, target_entity_id,
                                message_id, step);
        },
        [&connections](uuids::uuid source_entity_id,
                       uuids::uuid target_entity_id) {
            Connection::removeConnection(connections, source_entity_id,
                                         target_entity_id);
        },
        [&connections](uuids::uuid source_entity_id,
                       uuids::uuid target_entity_id, ConnectionStep step) {
            return Connection::isConnectedAtStep(connections, source_entity_id,
                                                 target_entity_id, step);
        });

    shared_ptr<Entity> entity_ptr = make_shared<Entity>(entity);
    print_message(entity_ptr->getName() + " [" +
                  to_string(entity_ptr->getId()) + "]");
    entity_ptr->printStorage({[&print_message](string message) {
        print_message(PrettyConsole::tab + message);
    }});
    return entity_ptr;
}

void GenericProtocol::sendMessage(shared_ptr<Entity> source,
                                  shared_ptr<Entity> target,
                                  string message_content,
                                  Message::Code message_code, Network &network,
                                  ostringstream &output_stream) {
    Message message = Message(uuid_generator, source->getId(), target->getId(),
                              message_content, message_code);

    printSendingMessageHeader(source, target, message_content, message_code,
                              network, output_stream);

    bool has_been_processed = network.receiveMessage(message);
    printSendingMessageFooter(has_been_processed, output_stream);
}

void GenericProtocol::printSendingMessageHeader(shared_ptr<Entity> source,
                                                shared_ptr<Entity> target,
                                                string message_content,
                                                Message::Code message_code,
                                                Network &network,
                                                ostringstream &output_stream) {
    output_stream << "Sending message" << endl;
    output_stream << PrettyConsole::tab
                  << "Source entity: " << source->getName() << " ["
                  << source->getId() << "]" << endl;
    output_stream << PrettyConsole::tab
                  << "Target entity: " << target->getName() << " ["
                  << target->getId() << "]" << endl;
    output_stream << PrettyConsole::tab
                  << "Message content: " << message_content << endl;
    output_stream << PrettyConsole::tab
                  << "Message code: " << Message::codeToString(message_code)
                  << endl;
    output_stream << PrettyConsole::tab << "Network: " << network.getName()
                  << endl;
}

void GenericProtocol::printSendingMessageFooter(bool has_been_processed,
                                                ostringstream &output_stream) {
    if (has_been_processed)
        output_stream << PrettyConsole::tab << "Message sent through network!"
                      << endl;
    else
        output_stream << PrettyConsole::tab
                      << "Message not sent through network!" << endl;

    output_stream << endl;
    cout << output_stream.str();
    output_stream.str("");
    cout.flush();
}
