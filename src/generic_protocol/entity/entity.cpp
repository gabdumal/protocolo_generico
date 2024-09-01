#include "entity.hpp"

#include <generic_protocol_constants.hpp>
#include <optional>

#include "message.hpp"
#include "util.hpp"

using namespace std;

/* Construction */

Entity::Entity(string name,
               shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    this->id = (*uuid_generator)();
    this->setName(name);
    this->storage = "";
    this->connections = unordered_map<uuids::uuid, shared_ptr<Connection>>();
    this->last_unacknowledged_message = nullopt;
}

Entity::~Entity() {}

/* Getters */

uuids::uuid Entity::getId() const { return this->id; }

string Entity::getName() const { return this->name; }

/* Setters */

void Entity::setName(string name) { this->name = name; }

/* Methods */

void Entity::printInformation(string information, ostream &output_stream,
                              PrettyConsole::Color color) const {
    string header =
        "Entity " + this->getName() + " [" + to_string(this->getId()) + "]";
    PrettyConsole::Decoration header_decoration(PrettyConsole::Color::WHITE,
                                                PrettyConsole::Color::CYAN,
                                                PrettyConsole::Format::BOLD);
    PrettyConsole::Decoration information_decoration(color);
    Util::printInformation(header, information, output_stream,
                           header_decoration, information_decoration);
}

void Entity::printStorage(function<void(string)> print_message) const {
    print_message("=== BEGIN ===");
    istringstream content_stream(this->storage);
    string line;
    while (getline(content_stream, line)) {
        print_message(line);
    }
    print_message("==== END ====");
}

optional<Entity::Connection> Entity::getConnection(
    uuids::uuid entity_id) const {
    auto connection = this->connections.find(entity_id);
    if (connection != this->connections.end()) {
        return *connection->second;
    }
    return nullopt;
}

bool Entity::isConnectedTo(uuids::uuid entity_id) const {
    return this->getConnection(entity_id).has_value();
}

bool Entity::canSendMessage(uuids::uuid message_id) const {
    // If there is no unacknowledged message, the entity can send a new message
    if (this->last_unacknowledged_message.has_value()) {
        // If the last unacknowledged message is the same as the message to be
        // sent, the entity can send it again
        if (this->last_unacknowledged_message.value().getId() == message_id) {
            return true;
        }
        return false;
    }
    return true;
}

bool Entity::canReceiveDataFrom(uuids::uuid entity_id) const {
    auto connection = this->connections.find(entity_id);
    return connection != this->connections.end() &&
           connection->second->ack_ack_syn_message_id.has_value();
}

void Entity::printMessageSendingInformation(Message &message,
                                            ostream &output_stream,
                                            PrettyConsole::Color color) const {
    if (GenericProtocolConstants::debug_information) {
        ostringstream message_content;
        message.print([&message_content](string line) {
            message_content << PrettyConsole::tab << line << endl;
        });

        optional<uuids::uuid> last_unacknowledged_message_id =
            this->last_unacknowledged_message.has_value()
                ? optional<uuids::uuid>(
                      this->last_unacknowledged_message.value().getId())
                : nullopt;

        string last_unacknowledged_message_id_as_string =
            last_unacknowledged_message_id.has_value()
                ? uuids::to_string(last_unacknowledged_message_id.value())
                : "N/A";

        string information = "Last unacknowledged message ID: [" +
                             last_unacknowledged_message_id_as_string +
                             "]\nTrying to send message\n" +
                             message_content.str();

        this->printInformation(information, cout, PrettyConsole::Color::YELLOW);
    }
}

bool Entity::sendMessage(Message &message) {
    if (!canSendMessage(message.getId())) return false;

    auto [should_send_message, should_lock_entity] =
        this->getSendingMessageConsequence(message);

    if (should_lock_entity) {
        this->last_unacknowledged_message = message;
    }

    return should_send_message;
}

bool Entity::processMessageBeingSent(Message &message) {
    Message::Code code = message.getCode();

    auto [should_send_message, should_lock_entity] =
        this->getSendingMessageConsequence(message);

    if (should_lock_entity) {
        this->last_unacknowledged_message = message;
    }

    return should_send_message;
}

Entity::MessageConsequence Entity::getSendingMessageConsequence(
    const Message &message) const {
    if (message.getCode() == Message::Code::NACK) {
        return {false, false};
    }
    auto connection_container =
        this->getConnection(message.getTargetEntityId());
    if (connection_container.has_value()) {
        return MessageConsequence{true, true};
    } else {
        if (message.getCode() == Message::Code::SYN)
            return MessageConsequence{true, true};
    }

    return {false, false};
}

Entity::Response Entity::receiveMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    if (GenericProtocolConstants::debug_information) {
        ostringstream message_content;
        message.print([&message_content](string line) {
            message_content << PrettyConsole::tab << line << endl;
        });

        optional<uuids::uuid> last_unacknowledged_message_id =
            this->last_unacknowledged_message.has_value()
                ? optional<uuids::uuid>(
                      this->last_unacknowledged_message.value().getId())
                : nullopt;

        string last_unacknowledged_message_id_as_string =
            last_unacknowledged_message_id.has_value()
                ? uuids::to_string(last_unacknowledged_message_id.value())
                : "N/A";

        string information = "Last unacknowledged message ID: [" +
                             last_unacknowledged_message_id_as_string +
                             "]\nReceive message\n" + message_content.str();

        this->printInformation(information, cout,
                               PrettyConsole::Color::MAGENTA);
    }

    optional<Message> response_message = nullopt;
    optional<uuids::uuid> id_from_message_possibly_acknowledged = nullopt;

    if (message.isCorrupted()) {
        response_message =
            Message(uuid_generator, this->id, message.getSourceEntityId(),
                    "NACK\n" + to_string(message.getId()), Message::Code::NACK);
    } else {
        switch (message.getCode()) {
            case Message::Code::SYN:
                response_message =
                    this->receiveSynMessage(message, uuid_generator);
                break;

            case Message::Code::FIN:
                response_message =
                    this->receiveFinMessage(message, uuid_generator);
                break;

            case Message::Code::ACK:
                response_message = this->receiveAckMessage(
                    message, id_from_message_possibly_acknowledged,
                    uuid_generator);
                break;

            case Message::Code::NACK:
                response_message =
                    this->receiveNackMessage(message, uuid_generator);
                break;

            case Message::Code::DATA:
                response_message =
                    this->receiveDataMessage(message, uuid_generator);
                break;
        }
    }

    return {response_message, id_from_message_possibly_acknowledged};
}

optional<Message> Entity::receiveSynMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto connection_container =
        this->getConnection(message.getSourceEntityId());
    if (!connection_container.has_value()) {
        Message ack_syn_message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "ACK-SYN\n" + to_string(message.getId()), Message::Code::ACK);

        // Still need to receive the ACK-ACK-SYN message
        auto connection = make_shared<Connection>(
            Connection{message.getId(), ack_syn_message.getId(), nullopt});

        this->connections.insert(pair<uuids::uuid, shared_ptr<Connection>>(
            message.getSourceEntityId(), connection));
        return ack_syn_message;
    } else {
        auto connection = connection_container.value();
        if (connection.syn_message_id == message.getId()) return nullopt;

        return Message(uuid_generator, this->id, message.getSourceEntityId(),
                       "NACK-SYN\n" + to_string(message.getId()),
                       Message::Code::NACK);
    }
}

optional<Message> Entity::receiveFinMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    if (this->isConnectedTo(message.getSourceEntityId()))
        this->connections.erase(message.getSourceEntityId());
    return nullopt;
}

optional<Message> Entity::receiveAckMessage(
    const Message &message,
    optional<uuids::uuid> &id_from_message_being_acknowledged,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto ack_type_container = message.getAckType();
    auto uuid_container = message.getIdFromMessageBeingAcknowledged();

    if (!ack_type_container.has_value()) return nullopt;
    Message::AckType ack_type = ack_type_container.value();
    if (uuid_container.has_value()) {
        uuids::uuid sent_message_id = uuid_container.value();

        // Unlock the entity to send a new message
        if (this->last_unacknowledged_message.has_value() &&
            this->last_unacknowledged_message.value().getId() ==
                sent_message_id) {
            id_from_message_being_acknowledged = sent_message_id;
            this->last_unacknowledged_message = nullopt;
        }

        if (ack_type == Message::AckType::ACK_SYN)
            return this->receiveAckSynMessage(message, sent_message_id,
                                              uuid_generator);
        else if (ack_type == Message::AckType::ACK_ACK_SYN)
            return this->receiveAckAckSynMessage(message, sent_message_id,
                                                 uuid_generator);

    } else {
        if (ack_type == Message::AckType::ACK_SYN)
            return Message(uuid_generator, this->id,
                           message.getSourceEntityId(),
                           "NACK-ACK-SYN\n" + to_string(message.getId()),
                           Message::Code::NACK);
        else if (ack_type == Message::AckType::ACK_ACK_SYN)
            return Message(uuid_generator, this->id,
                           message.getSourceEntityId(),
                           "NACK-ACK-ACK-SYN\n" + to_string(message.getId()),
                           Message::Code::NACK);
    }
    return nullopt;
}

optional<Message> Entity::receiveAckSynMessage(
    const Message &message, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    uuids::uuid syn_message_id = sent_message_id;

    Message ack_ack_syn_message(
        uuid_generator, this->id, message.getSourceEntityId(),
        "ACK-ACK-SYN\n" + to_string(message.getId()), Message::Code::ACK);

    auto connection = make_shared<Connection>(Connection{
        syn_message_id,
        message.getId(),
        ack_ack_syn_message.getId(),
    });

    this->connections.insert(pair<uuids::uuid, shared_ptr<Connection>>(
        message.getSourceEntityId(), connection));

    return ack_ack_syn_message;
}

optional<Message> Entity::receiveAckAckSynMessage(
    const Message &message, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    uuids::uuid ack_syn_message_id = sent_message_id;

    auto connection = this->connections.find(message.getSourceEntityId());

    if (this->isConnectedTo(message.getSourceEntityId()) &&
        connection->second->ack_syn_message_id == ack_syn_message_id) {
        // Update connection
        connection->second->ack_ack_syn_message_id = message.getId();
        return nullopt;
    }

    return Message(uuid_generator, this->id, message.getSourceEntityId(),
                   "NACK-ACK-ACK-SYN\n" + to_string(message.getId()),
                   Message::Code::NACK);
}

optional<Message> Entity::receiveNackMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return nullopt;
}

optional<Message> Entity::receiveDataMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    if (this->isConnectedTo(message.getSourceEntityId())) {
        this->storage += message.getContent() + "\n";

        return Message(uuid_generator, this->id, message.getSourceEntityId(),
                       "ACK\n" + to_string(message.getId()),
                       Message::Code::ACK);
    }

    return Message(uuid_generator, this->id, message.getSourceEntityId(),
                   "NACK\n" + to_string(message.getId()), Message::Code::NACK);
}
