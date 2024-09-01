#include "connection.hpp"

#include <memory>

#include "entity.hpp"

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
}

bool Connection::isConnectedAtStep(
    map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
    uuids::uuid source_entity_id, uuids::uuid target_entity_id,
    ConnectionStep step) {
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};
    if (connections.find(key) == connections.end()) {
        return false;
    }
    shared_ptr<Connection> connection = connections[key];
    return connection->isConnectedAtStep(step);
}

void Connection::connect(
    map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
    uuids::uuid source_entity_id, uuids::uuid target_entity_id,
    uuids::uuid message_id, ConnectionStep step) {
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};

    if (connections.find(key) != connections.end()) {
        shared_ptr<Connection> connection = connections[key];
        connection->connect(message_id, step);
    }
}

void Connection::removeConnection(
    map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
    uuids::uuid source_entity_id, uuids::uuid target_entity_id) {
    pair<uuids::uuid, uuids::uuid> key = {source_entity_id, target_entity_id};
    if (connections.find(key) != connections.end()) {
        shared_ptr<Connection> connection = connections[key];
        connection->removeConnection();
    }
}
