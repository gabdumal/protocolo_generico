#include "connection.hpp"

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
    map<pair<shared_ptr<Entity>, shared_ptr<Entity>>, Connection> &connections,
    shared_ptr<Entity> source, shared_ptr<Entity> target, ConnectionStep step) {
    auto connection = connections.find(make_pair(source, target));
    if (connection == connections.end()) {
        return false;
    }
    return connection->second.isConnectedAtStep(step);
}

void Connection::connect(
    map<pair<shared_ptr<Entity>, shared_ptr<Entity>>, Connection> &connections,
    shared_ptr<Entity> source, shared_ptr<Entity> target,
    uuids::uuid message_id, ConnectionStep step) {
    auto connection = connections.find(make_pair(source, target));
    if (connection == connections.end()) {
        connections.insert(make_pair(make_pair(source, target), Connection()));
        connection = connections.find(make_pair(source, target));
    }
    connection->second.connect(message_id, step);
}

void Connection::removeConnection(
    map<pair<shared_ptr<Entity>, shared_ptr<Entity>>, Connection> &connections,
    shared_ptr<Entity> source, shared_ptr<Entity> target) {
    auto connection = connections.find(make_pair(source, target));
    if (connection != connections.end()) {
        connection->second.removeConnection();
    }
}
