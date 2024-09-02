#include "connection.hpp"

#include <memory>

#include "entity.hpp"
#include "generic_protocol_constants.hpp"
#include "uuid.h"

using namespace std;

bool Connection::isConnectedAtStep(ConnectionStep step) {
    switch (step) {
        case ConnectionStep::NONE:
            return true;
        case ConnectionStep::SYN:
            return this->syn_message_id.has_value();
        case ConnectionStep::ACK_SYN:
            return this->ack_syn_message_id.has_value();
        case ConnectionStep::ACK_ACK_SYN:
            return this->ack_ack_syn_message_id.has_value();
        default:
            return false;
    }
}

void Connection::connect(uuids::uuid message_id, ConnectionStep step) {
    switch (step) {
        case ConnectionStep::SYN:
            this->syn_message_id = message_id;
            break;
        case ConnectionStep::ACK_SYN:
            this->ack_syn_message_id = message_id;
            break;
        case ConnectionStep::ACK_ACK_SYN:
            this->ack_ack_syn_message_id = message_id;
            break;
        default:
            break;
    }
}

void Connection::removeConnection() {
    this->syn_message_id = nullopt;
    this->ack_syn_message_id = nullopt;
    this->ack_ack_syn_message_id = nullopt;
    while (!this->unconfirmed_sent_packages->empty())
        this->unconfirmed_sent_packages->pop();
}

bool Connection::canSendPackage() {
    if (!this->isConnectedAtStep(ConnectionStep::ACK_ACK_SYN)) return false;
    if (this->unconfirmed_sent_packages->size() >= this->buffer_size)
        return false;
    return true;
}

bool Connection::canStoreData(uuids::uuid message_id) {
    if (!this->isConnectedAtStep(ConnectionStep::ACK_ACK_SYN)) return false;
    auto front = this->unconfirmed_sent_packages->front();
    if (front != message_id) return false;
    return true;
}

void Connection::connect(
    shared_ptr<ConnectionsMap> connections,
    ConnectFunctionParameters connect_function_parameters) {
    if (connections == nullptr) return;

    tuple<uuids::uuid, uuids::uuid, uuids::uuid, ConnectionStep> parameters =
        connect_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);
    uuids::uuid message_id = get<2>(parameters);
    ConnectionStep step = get<3>(parameters);

    auto &connections_obj = *connections;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections_obj.find(key) == connections_obj.end()) {
        auto connection = make_shared<Connection>(
            GenericProtocolConstants::connection_buffer_size);
        connection->connect(message_id, step);
        connections_obj.insert({key, connection});

    } else {
        auto connection = connections_obj[key];
        connection->connect(message_id, step);
    }
}

void Connection::removeConnection(
    shared_ptr<ConnectionsMap> connections,
    RemoveConnectionFunctionParameters remove_connection_function_parameters) {
    if (connections == nullptr) return;

    tuple<uuids::uuid, uuids::uuid> parameters =
        remove_connection_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);

    auto &connections_obj = *connections;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections_obj.find(key) != connections_obj.end()) {
        auto connection = connections_obj[key];
        connection->removeConnection();
        connections_obj.erase(key);
    }
}

bool Connection::isConnectedAtStep(
    shared_ptr<ConnectionsMap> connections,
    IsConnectedAtStepFunctionParameters
        is_connected_at_step_function_parameters) {
    if (connections == nullptr) return false;

    tuple<uuids::uuid, uuids::uuid, ConnectionStep> parameters =
        is_connected_at_step_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);
    ConnectionStep step = get<2>(parameters);

    auto &connections_obj = *connections;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections_obj.find(key) != connections_obj.end()) {
        auto connection = connections_obj[key];
        auto is_connected_at_step = connection->isConnectedAtStep(step);
        return is_connected_at_step;
    }

    return false;
}

bool Connection::canSendPackage(
    shared_ptr<ConnectionsMap> connections,
    CanSendPackageFunctionParameters can_send_data_function_parameters) {
    if (connections == nullptr) return false;

    tuple<uuids::uuid, uuids::uuid> parameters =
        can_send_data_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);

    auto &connections_obj = *connections;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections_obj.find(key) != connections_obj.end()) {
        auto connection = connections_obj[key];
        auto can_send_data = connection->canSendPackage();
        return can_send_data;
    }

    return false;
}

bool Connection::canStoreData(
    shared_ptr<ConnectionsMap> connections,
    CanStoreDataFunctionParameters can_store_data_function_parameters) {
    if (connections == nullptr) return false;

    tuple<uuids::uuid, uuids::uuid, uuids::uuid> parameters =
        can_store_data_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);
    uuids::uuid message_id = get<2>(parameters);

    auto &connections_obj = *connections;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections_obj.find(key) != connections_obj.end()) {
        auto connection = connections_obj[key];
        auto is_fully_connected =
            connection->isConnectedAtStep(ConnectionStep::ACK_ACK_SYN);
        if (!is_fully_connected) return false;
        auto can_store_data = connection->canStoreData(message_id);
        return can_store_data;
    }

    return false;
}
