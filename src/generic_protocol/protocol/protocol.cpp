#include "protocol.hpp"

#include "entity.hpp"

using namespace std;

Protocol::Protocol(shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    this->uuid_generator = uuid_generator;
    this->entities = make_shared<EntitiesList>();
    this->connections = make_shared<ConnectionsMap>();
}

Protocol::~Protocol() {}

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

    shared_ptr<Entity> entity = make_shared<Entity>(
        this->uuid_generator->operator()(), name, connect_function,
        remove_connection_function, is_connected_at_step_function,
        can_send_package_function, can_store_data_function);

    printMessage(entity->getName() + " [" + to_string(entity->getId()) + "]",
                 output_stream);

    entity->printStorage({[this, &output_stream](string message) {
        this->printMessage(PrettyConsole::tab + message, output_stream);
    }});

    this->entities->push_back(entity);

    return entity_id;
}

/* Static methods */
void Protocol::printMessage(string message, ostringstream &output_stream) {
    output_stream << message << endl;
}