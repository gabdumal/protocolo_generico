#include <message.hpp>
#include <sstream>

using namespace std;

/* Auxiliary */
string Message::codeToString(Message::Code code) {
    switch (code) {
        case Message::Code::SYN:
            return "SYN";
        case Message::Code::FIN:
            return "FIN";
        case Message::Code::ACK:
            return "ACK";
        case Message::Code::NACK:
            return "NACK";
        case Message::Code::DATA:
            return "DATA";
        default:
            return "UNKNOWN";
    }
}

string Message::codeVariantToString(Message::CodeVariant code_variant) {
    switch (code_variant) {
        case Message::CodeVariant::ACK:
            return "ACK";
        case Message::CodeVariant::ACK_SYN:
            return "ACK_SYN";
        case Message::CodeVariant::ACK_ACK_SYN:
            return "ACK_ACK_SYN";
        case Message::CodeVariant::ACK_ACK_ACK_SYN:
            return "ACK_ACK_ACK_SYN";
        case Message::CodeVariant::ACK_FIN:
            return "ACK_FIN";
        case Message::CodeVariant::ACK_ACK_FIN:
            return "ACK_ACK_FIN";
        case Message::CodeVariant::NACK:
            return "NACK";
        case Message::CodeVariant::NACK_SYN:
            return "NACK_SYN";
        case Message::CodeVariant::NACK_ACK_SYN:
            return "NACK_ACK_SYN";
        case Message::CodeVariant::NACK_ACK_ACK_SYN:
            return "NACK_ACK_ACK_SYN";
        case Message::CodeVariant::NACK_FIN:
            return "NACK_FIN";
        default:
            return "UNKNOWN";
    }
}

/* Getters */

uuids::uuid Message::getId() const { return this->id; }

uuids::uuid Message::getSourceEntityId() const {
    return this->source_entity_id;
}

uuids::uuid Message::getTargetEntityId() const {
    return this->target_entity_id;
}

Message::Code Message::getCode() const { return this->code; }

optional<Message::CodeVariant> Message::getCodeVariant() const {
    return this->code_variant;
}

optional<uuids::uuid> Message::getIdFromMessageBeingAcknowledged() const {
    return this->id_from_message_being_acknowledged;
}

string Message::getContent() const { return this->content; }

/* Setters */

void Message::setCodeVariant(Message::CodeVariant code_variant) {
    this->code_variant = code_variant;
}

void Message::setIdFromMessageBeingAcknowledged(uuids::uuid id_from_message) {
    this->id_from_message_being_acknowledged = id_from_message;
}

/* Methods */

void Message::print(std::function<void(std::string)> print_information) const {
    print_information("ID: " + uuids::to_string(this->getId()));
    print_information("Source entity ID: " +
                      uuids::to_string(this->getSourceEntityId()));
    print_information("Target entity ID: " +
                      uuids::to_string(this->getTargetEntityId()));
    print_information("Code: " + codeToString(this->getCode()));
    print_information("Code variant: " +
                      (this->getCodeVariant().has_value()
                           ? codeVariantToString(this->getCodeVariant().value())
                           : "NONE"));
    print_information(
        "ID from message being acknowledged: " +
        (this->getIdFromMessageBeingAcknowledged().has_value()
             ? uuids::to_string(
                   this->getIdFromMessageBeingAcknowledged().value())
             : "NONE"));
    print_information("=== BEGIN ===");
    std::istringstream content_stream(this->getContent());
    std::string line;
    while (std::getline(content_stream, line)) {
        print_information(line);
    }
    print_information("==== END ====");
}
