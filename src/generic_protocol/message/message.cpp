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

/* Construction */

Message::Message(shared_ptr<uuids::uuid_random_generator> uuid_generator,
                 uuids::uuid source_entity_id, uuids::uuid target_entity_id,
                 Code code, optional<CodeVariant> code_variant,
                 optional<uuids::uuid> id_from_message_being_acknowledged,
                 string content) {
    this->id = (*uuid_generator)();
    this->source_entity_id = source_entity_id;
    this->target_entity_id = target_entity_id;
    this->code = code;
    this->code_variant = code_variant;
    this->id_from_message_being_acknowledged =
        id_from_message_being_acknowledged;
    this->content = content;
}

Message::~Message() {}

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

void Message::print(std::function<void(std::string)> print_message) const {
    print_message("ID: " + uuids::to_string(this->getId()));
    print_message("Source entity ID: " +
                  uuids::to_string(this->getSourceEntityId()));
    print_message("Target entity ID: " +
                  uuids::to_string(this->getTargetEntityId()));
    print_message("Code: " + codeToString(this->getCode()));
    print_message("=== BEGIN ===");
    std::istringstream content_stream(this->getContent());
    std::string line;
    while (std::getline(content_stream, line)) {
        print_message(line);
    }
    print_message("==== END ====");
}
