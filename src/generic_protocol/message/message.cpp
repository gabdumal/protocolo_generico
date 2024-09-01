#include <message.hpp>
#include <sstream>

using namespace std;

/* Auxiliary */
string codeToString(Code code) {
    switch (code) {
        case Code::SYN:
            return "SYN";
        case Code::FIN:
            return "FIN";
        case Code::ACK:
            return "ACK";
        case Code::NACK:
            return "NACK";
        case Code::DATA:
            return "DATA";
        default:
            return "UNKNOWN";
    }
}

/* Construction */

Message::Message(shared_ptr<uuids::uuid_random_generator> uuid_generator,
                 uuids::uuid source_entity_id, uuids::uuid target_entity_id,
                 string content, Code code) {
    this->id = (*uuid_generator)();
    this->source_entity_id = source_entity_id;
    this->target_entity_id = target_entity_id;
    this->content = content;
    this->corrupted = false;
    this->code = code;
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

string Message::getContent() const { return this->content; }

bool Message::isCorrupted() const { return this->corrupted; }

Code Message::getCode() const { return this->code; }

/* Setters */

void Message::setCorrupted(bool is_corrupted) {
    this->corrupted = is_corrupted;
}

/* Methods */

void Message::print(std::function<void(std::string)> print_message) const {
    print_message("ID: " + uuids::to_string(this->getId()));
    print_message("Source entity ID: " +
                  uuids::to_string(this->getSourceEntityId()));
    print_message("Target entity ID: " +
                  uuids::to_string(this->getTargetEntityId()));
    print_message("Code: " + codeToString(this->getCode()));
    print_message("Corrupted: " +
                  std::string(this->isCorrupted() ? "YES" : "NO"));
    print_message("=== BEGIN ===");
    std::istringstream content_stream(this->getContent());
    std::string line;
    while (std::getline(content_stream, line)) {
        print_message(line);
    }
    print_message("==== END ====");
}
