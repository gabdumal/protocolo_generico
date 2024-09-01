#include "entity.hpp"

#include <generic_protocol_constants.hpp>

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

bool Entity::isConnectedTo(uuids::uuid entity_id) const {
    return this->connections.find(entity_id) != this->connections.end();
}

bool Entity::canSendMessage() const {
    // If there is no unacknowledged message, the entity can send a new message
    return !this->last_unacknowledged_message.has_value();
}

bool Entity::canReceiveDataFrom(uuids::uuid entity_id) const {
    auto connection = this->connections.find(entity_id);
    return connection != this->connections.end() &&
           connection->second->ack_ack_syn_message_id.has_value();
}

bool Entity::sendMessage(Message &message) {
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

    if (!canSendMessage()) return false;

    if (isConnectedTo(message.getTargetEntityId())) {
        this->last_unacknowledged_message = message;
        return true;
    } else if (message.getCode() == Message::Code::SYN) {
        this->last_unacknowledged_message = message;
        return true;
    }

    return false;
}

optional<Message> Entity::receiveMessage(
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

    if (message.isCorrupted()) {
        return Message(uuid_generator, this->id, message.getSourceEntityId(),
                       "NACK\n" + to_string(message.getId()),
                       Message::Code::NACK);
    }

    switch (message.getCode()) {
        case Message::Code::SYN:
            return this->receiveSynMessage(message, uuid_generator);

        case Message::Code::FIN:
            return this->receiveFinMessage(message, uuid_generator);

        case Message::Code::ACK:
            return this->receiveAckMessage(message, uuid_generator);

        case Message::Code::NACK:
            return this->receiveNackMessage(message, uuid_generator);

        case Message::Code::DATA:
            return this->receiveDataMessage(message, uuid_generator);
    }

    return nullopt;
}

optional<Message> Entity::receiveSynMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    if (!this->isConnectedTo(message.getSourceEntityId())) {
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
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    string first_line = Util::getLineContent(1, message.getContent());
    auto uuid_container = message.getIdFromMessageBeingAcknowledged();

    if (uuid_container.has_value()) {
        uuids::uuid sent_message_id = uuid_container.value();

        // Unlock the entity to send a new message
        if (this->last_unacknowledged_message.has_value() &&
            this->last_unacknowledged_message.value().getId() ==
                sent_message_id) {
            this->last_unacknowledged_message = nullopt;
        }

        if (first_line == "ACK-SYN")
            return this->receiveAckSynMessage(message, sent_message_id,
                                              uuid_generator);
        else if (first_line == "ACK-ACK-SYN")
            return this->receiveAckAckSynMessage(message, sent_message_id,
                                                 uuid_generator);
    } else {
        if (first_line == "ACK-SYN")
            return Message(uuid_generator, this->id,
                           message.getSourceEntityId(),
                           "NACK-ACK-SYN\n" + to_string(message.getId()),
                           Message::Code::NACK);
        else if (first_line == "ACK-ACK-SYN")
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
