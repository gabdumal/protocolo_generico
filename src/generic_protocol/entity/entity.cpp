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

/* Connection */

void Entity::connect(
    InternalConnectFunctionParameters connect_function_parameters) {
    ConnectFunctionParameters parameters = {
        this->id, get<0>(connect_function_parameters),
        get<1>(connect_function_parameters),
        get<2>(connect_function_parameters)};
    this->connect_function->operator()(parameters);
}

void Entity::removeConnection(InternalRemoveConnectionFunctionParameters
                                  remove_connection_function_parameters) {
    RemoveConnectionFunctionParameters parameters = {
        this->id, get<0>(remove_connection_function_parameters)};
    this->remove_connection_function->operator()(parameters);
}

bool Entity::isConnectedAtStep(
    InternalIsConnectedAtStepFunctionParameters
        is_connected_at_step_function_parameters) const {
    IsConnectedAtStepFunctionParameters parameters = {
        this->id, get<0>(is_connected_at_step_function_parameters),
        get<1>(is_connected_at_step_function_parameters)};
    auto is_connected_at_step =
        this->is_connected_at_step_function->operator()(parameters);
    return is_connected_at_step;
}

/* Methods */

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

// bool Entity::shouldBeConfirmed(Message &message) const {
//     auto code = message.getCode();
//     if (code == Message::Code::NACK) return false;
//     auto is_connected_at_step = this->isConnectedAtStep(
//         {message.getSourceEntityId(), ConnectionStep::SYN});
//     if (is_connected_at_step)
//         return true;
//     else if (code == Message::Code::SYN)
//         return true;
//     return false;
// }

// Entity::MessageConsequence Entity::getSendingMessageConsequence(
//     const Message &message) const {
//     if (message.getCode() == Message::Code::NACK) {
//         return {false, false};
//     }
//     if (this->isConnectedAtStep(
//             {message.getSourceEntityId(), ConnectionStep::NONE})) {
//         return MessageConsequence{true, true};
//     } else {
//         if (message.getCode() == Message::Code::SYN)
//             return MessageConsequence{true, true};
//     }

//     return {false, false};
// }

bool Entity::sendMessage(Message message, bool should_be_confirmed) {
    if (!canSendMessage(message.getId())) return false;
    if (should_be_confirmed) this->last_unacknowledged_message = message;
    return true;
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
                             operation + message_content.str();

        this->printInformation(information, cout,
                               is_sending ? PrettyConsole::Color::YELLOW
                                          : PrettyConsole::Color::MAGENTA);
    }
}
