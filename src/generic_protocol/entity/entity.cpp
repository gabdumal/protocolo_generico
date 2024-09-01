#include "entity.hpp"

#include <generic_protocol_constants.hpp>
#include <optional>

#include "message.hpp"
#include "util.hpp"

using namespace std;

/* Getters */

uuids::uuid Entity::getId() const { return this->id; }

string Entity::getName() const { return this->name; }

/* Setters */

void Entity::setName(string name) { this->name = name; }

/* Methods */

void Entity::connect(uuids::uuid target_entity_id, uuids::uuid message_id,
                     ConnectionStep step) {
    this->connect_function(this->id, target_entity_id, message_id, step);
}

void Entity::removeConnection(uuids::uuid target_entity_id) {
    this->remove_connection_function(this->id, target_entity_id);
}

bool Entity::isConnectedAtStep(uuids::uuid target_entity_id,
                               ConnectionStep step) const {
    return this->is_connected_at_step_function(this->id, target_entity_id,
                                               step);
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

Entity::MessageConsequence Entity::getSendingMessageConsequence(
    const Message &message) const {
    if (message.getCode() == Message::Code::NACK) {
        return {false, false};
    }
    if (this->isConnectedAtStep(message.getSourceEntityId(),
                                ConnectionStep::ACK_ACK_SYN)) {
        return MessageConsequence{true, true};
    } else {
        if (message.getCode() == Message::Code::SYN)
            return MessageConsequence{true, true};
    }

    return {false, false};
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

void Entity::printMessageInformation(const Message &message,
                                     ostream &output_stream,
                                     bool is_sending) const {
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

        string operation =
            is_sending ? "Trying to send message\n" : "Received message\n";
        string information = "Last unacknowledged message ID: [" +
                             last_unacknowledged_message_id_as_string + "]\n" +
                             "Last received message ID: [" + operation +
                             message_content.str();

        this->printInformation(information, cout,
                               is_sending ? PrettyConsole::Color::YELLOW
                                          : PrettyConsole::Color::MAGENTA);
    }
}
