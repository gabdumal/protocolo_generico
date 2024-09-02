#include "connection.hpp"

#include <memory>

#include "entity.hpp"
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
    this->last_data_message_id = nullopt;
}

bool Connection::canStoreData(optional<uuids::uuid> message_id_container) {
    if (this->last_data_message_id.has_value()) {
        if (message_id_container.has_value()) {
            return this->last_data_message_id.value() ==
                   message_id_container.value();
        } else {
            return false;
        }
    } else {
        if (message_id_container.has_value()) {
            return false;
        } else {
            return true;
        }
    }
}

void Connection::setLastDataMessageId(uuids::uuid message_id) {
    this->last_data_message_id = message_id;
}

void Connection::connect(
    ConnectionsMapPointer connections_ptr,
    ConnectFunctionParameters connect_function_parameters) {
    if (connections_ptr == nullptr) return;

    tuple<uuids::uuid, uuids::uuid, uuids::uuid, ConnectionStep> parameters =
        connect_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);
    uuids::uuid message_id = get<2>(parameters);
    ConnectionStep step = get<3>(parameters);

    auto &connections = *connections_ptr;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections.find(key) == connections.end()) {
        auto connection = make_shared<Connection>();
        connection->connect(message_id, step);
        connections.insert({key, connection});

    } else {
        auto connection = connections[key];
        connection->connect(message_id, step);
    }
}

void Connection::removeConnection(
    ConnectionsMapPointer connections_ptr,
    RemoveConnectionFunctionParameters remove_connection_function_parameters) {
    if (connections_ptr == nullptr) return;

    tuple<uuids::uuid, uuids::uuid> parameters =
        remove_connection_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);

    auto &connections = *connections_ptr;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections.find(key) != connections.end()) {
        auto connection = connections[key];
        connection->removeConnection();
        connections.erase(key);
    }
}

bool Connection::isConnectedAtStep(
    ConnectionsMapPointer connections_ptr,
    IsConnectedAtStepFunctionParameters
        is_connected_at_step_function_parameters) {
    if (connections_ptr == nullptr) return false;

    tuple<uuids::uuid, uuids::uuid, ConnectionStep> parameters =
        is_connected_at_step_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);
    ConnectionStep step = get<2>(parameters);

    auto &connections = *connections_ptr;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections.find(key) != connections.end()) {
        auto connection = connections[key];
        auto is_connected_at_step = connection->isConnectedAtStep(step);
        return is_connected_at_step;
    }

    return false;
}

bool Connection::canStoreData(
    ConnectionsMapPointer connections_ptr,
    CanStoreDataFunctionParameters can_store_data_function_parameters) {
    if (connections_ptr == nullptr) return false;

    tuple<uuids::uuid, uuids::uuid, optional<uuids::uuid>> parameters =
        can_store_data_function_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);
    optional<uuids::uuid> message_id_container = get<2>(parameters);

    auto &connections = *connections_ptr;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections.find(key) != connections.end()) {
        auto connection = connections[key];
        auto is_fully_connected =
            connection->isConnectedAtStep(ConnectionStep::ACK_ACK_SYN);
        if (!is_fully_connected) return false;
        auto can_store_data = connection->canStoreData(message_id_container);
        return can_store_data;
    }

    return false;
}

void Connection::setLastDataMessageId(ConnectionsMapPointer connections_ptr,
                                      SetLastDataMessageIdFunctionParameters
                                          set_last_data_message_id_parameters) {
    if (connections_ptr == nullptr) return;

    tuple<uuids::uuid, uuids::uuid, uuids::uuid> parameters =
        set_last_data_message_id_parameters;
    uuids::uuid source_entity_id = get<0>(parameters);
    uuids::uuid target_entity_id = get<1>(parameters);
    uuids::uuid message_id = get<2>(parameters);

    auto &connections = *connections_ptr;
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections.find(key) != connections.end()) {
        auto connection = connections[key];
        connection->setLastDataMessageId(message_id);
    }
}
