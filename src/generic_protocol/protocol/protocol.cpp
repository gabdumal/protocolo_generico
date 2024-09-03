#include "protocol.hpp"

#include "entity.hpp"
#include "message.hpp"
#include "package.hpp"

using namespace std;

/* Construction */

Protocol::Protocol(shared_ptr<uuids::uuid_random_generator> uuid_generator,
                   string network_name) {
    this->uuid_generator = uuid_generator;
    this->entities = make_shared<EntitiesList>();
    this->connections = make_shared<ConnectionsMap>();
    this->network = make_unique<Network>(
        this->uuid_generator, network_name, [this](uuids::uuid entity_id) {
            return this->getEntityById(entity_id);
        });
}

Protocol::~Protocol() {
    this->entities->clear();
    this->connections->clear();
    this->network->joinThreads();
}

/* Getters */

shared_ptr<Entity> Protocol::getEntityById(uuids::uuid entity_id) {
    if (this->entities == nullptr) return nullptr;
    for (auto entity : *this->entities) {
        if (entity->getId() == entity_id) {
            return entity;
        }
    }
    return nullptr;
}

/* Methods */

uuids::uuid Protocol::createEntity(string name, ostringstream &output_stream) {
    uuids::uuid entity_id = this->uuid_generator->operator()();

    auto connect_lambda = [this](uuids::uuid source_entity_id,
                                 uuids::uuid target_entity_id,
                                 uuids::uuid message_id, ConnectionStep step) {
        Connection::connect(
            this->connections,
            {source_entity_id, target_entity_id, message_id, step});
    };
    ConnectFunction connect_function = make_shared<
        function<void(ConnectFunctionParameters connect_function_parameters)>>(
        [connect_lambda](ConnectFunctionParameters params) {
            apply(connect_lambda, params);
        });

    auto remove_connection_lambda = [this](uuids::uuid source_entity_id,
                                           uuids::uuid target_entity_id) {
        Connection::removeConnection(this->connections,
                                     {source_entity_id, target_entity_id});
    };
    RemoveConnectionFunction remove_connection_function =
        make_shared<function<void(RemoveConnectionFunctionParameters
                                      remove_connection_function_parameters)>>(
            [remove_connection_lambda](
                RemoveConnectionFunctionParameters params) {
                apply(remove_connection_lambda, params);
            });

    auto is_connected_at_step_lambda = [this](uuids::uuid source_entity_id,
                                              uuids::uuid target_entity_id,
                                              ConnectionStep step) {
        return Connection::isConnectedAtStep(
            this->connections, {source_entity_id, target_entity_id, step});
    };
    IsConnectedAtStepFunction is_connected_at_step_function = make_shared<
        function<bool(IsConnectedAtStepFunctionParameters
                          is_connected_at_step_function_parameters)>>(
        [is_connected_at_step_lambda](
            IsConnectedAtStepFunctionParameters params) {
            return apply(is_connected_at_step_lambda, params);
        });

    auto can_send_package_lambda = [this](uuids::uuid source_entity_id,
                                          uuids::uuid target_entity_id) {
        return Connection::canSendPackage(this->connections,
                                          {source_entity_id, target_entity_id});
    };
    CanSendPackageFunction can_send_package_function =
        make_shared<function<bool(CanSendPackageFunctionParameters
                                      can_send_package_function_parameters)>>(
            [can_send_package_lambda](CanSendPackageFunctionParameters params) {
                return apply(can_send_package_lambda, params);
            });

    auto can_store_data_lambda = [this](uuids::uuid source_entity_id,
                                        uuids::uuid target_entity_id,
                                        uuids::uuid message_id) {
        return Connection::canStoreData(
            this->connections,
            {source_entity_id, target_entity_id, message_id});
    };
    CanStoreDataFunction can_store_data_function = make_shared<function<bool(
        CanStoreDataFunctionParameters can_store_data_function_parameters)>>(
        [can_store_data_lambda](CanStoreDataFunctionParameters params) {
            return apply(can_store_data_lambda, params);
        });

    auto dequeue_package_lambda = [this](uuids::uuid source_entity_id,
                                         uuids::uuid target_entity_id) {
        Connection::dequeuePackage(this->connections,
                                   {source_entity_id, target_entity_id});
    };
    DequeuePackageFunction dequeue_package_function = make_shared<function<void(
        DequeuePackageFunctionParameters dequeue_package_function_parameters)>>(
        [dequeue_package_lambda](DequeuePackageFunctionParameters params) {
            apply(dequeue_package_lambda, params);
        });

    shared_ptr<Entity> entity = make_shared<Entity>(
        entity_id, name, connect_function, remove_connection_function,
        is_connected_at_step_function, can_send_package_function,
        can_store_data_function, dequeue_package_function);

    printInformation(
        entity->getName() + " [" + to_string(entity->getId()) + "]",
        output_stream);

    entity->printStorage({[this, &output_stream](string message) {
        this->printInformation(PrettyConsole::tab + message, output_stream);
    }});

    this->entities->push_back(entity);

    return entity_id;
}

shared_ptr<Connection> Protocol::connectEntities(
    shared_ptr<Entity> source_entity, shared_ptr<Entity> target_entity,
    ostringstream &output_stream) {
    if (source_entity == nullptr) {
        printInformation("Source entity not found", output_stream);
        return nullptr;
    }
    if (target_entity == nullptr) {
        printInformation("Target entity not found", output_stream);
        return nullptr;
    }

    auto connection_it = this->connections->find(
        {source_entity->getId(), target_entity->getId()});
    shared_ptr<Connection> connection = nullptr;

    if (connection_it != this->connections->end()) {
        connection = connection_it->second;
        if (connection->isConnectedAtStep(ConnectionStep::ACK_ACK_SYN)) {
            printInformation("Connection already exists", output_stream);
            return connection;
        }
    }

    Message syn_message(this->uuid_generator, source_entity->getId(),
                        target_entity->getId(), Message::Code::SYN, nullopt,
                        nullopt, "");
    Package syn_package(syn_message, true, 0);

    this->network->receivePackage(syn_package);

    unsigned int attempts = 0;
    while (attempts < GenericProtocolConstants::max_attempts_to_connect) {
        auto connection_it = this->connections->find(
            {source_entity->getId(), target_entity->getId()});
        if (connection_it != this->connections->end()) {
            connection = connection_it->second;
            if (connection->isConnectedAtStep(ConnectionStep::ACK_ACK_SYN)) {
                printInformation("Entities connected", output_stream);
                return connection;
            }
        }
        this_thread::sleep_for(
            GenericProtocolConstants::interval_to_check_connections);
        attempts++;
    }

    printInformation("Connection failed!", output_stream);
    return nullptr;
}

void Protocol::sendData(uuids::uuid source_entity_id,
                        uuids::uuid target_entity_id, deque<string> contents,
                        ostringstream &output_stream) {
    shared_ptr<Entity> source_entity = this->getEntityById(source_entity_id);
    if (source_entity == nullptr) {
        printInformation("Source entity not found", output_stream);
        return;
    }
    shared_ptr<Entity> target_entity = this->getEntityById(target_entity_id);
    if (target_entity == nullptr) {
        printInformation("Target entity not found", output_stream);
        return;
    }

    shared_ptr<Connection> connection =
        this->connectEntities(source_entity, target_entity, output_stream);
    if (connection == nullptr) {
        this->printInformation("Could not send data!", output_stream);
    }

    unsigned int sequence_number = 0;
    for (auto content : contents) {
        unsigned int attempts = 0;
        connection->lockQueue();
        while (!connection->canSendPackage() &&
               attempts < GenericProtocolConstants::max_attempts_to_send_data) {
            this_thread::sleep_for(
                GenericProtocolConstants::interval_to_send_data);
        }
        if (attempts == GenericProtocolConstants::max_attempts_to_send_data) {
            printInformation("Could not send data!", output_stream);
            return;
        }

        sequence_number++;
        Message message =
            Message(this->uuid_generator, source_entity_id, target_entity_id,
                    Message::Code::DATA, nullopt, nullopt, content);
        Package package(message, true, sequence_number);

        connection->enqueuePackage(message.getId());
        this->network->receivePackage(package);

        connection->unlockQueue();

        package.print({[this, &output_stream](string information) {
            this->printInformation(PrettyConsole::tab + information,
                                   output_stream);
        }});
        output_stream << endl;
        cout << output_stream.str();
        output_stream.str("");
    }

    this->network->joinThreads();
    printInformation("Messages sent", output_stream);
}

/* Static methods */
void Protocol::printInformation(string information,
                                ostringstream &output_stream) {
    output_stream << information << endl;
}

void Protocol::printEntitiesStorage(ostringstream &output_stream) {
    output_stream << "Entities' storage" << endl;

    for (auto entity : *this->entities) {
        output_stream << entity->getName() << " [" << entity->getId() << "]"
                      << endl;
        entity->printStorage({[this, &output_stream](string message) {
            this->printInformation(PrettyConsole::tab + message, output_stream);
        }});
    }
}
