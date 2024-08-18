#include <message.hpp>
#include <iostream>

using namespace std;

/* Auxiliary */
string codeToString(Code code)
{
    switch (code)
    {
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

Message::Message(uuids::uuid_random_generator *uuidGenerator, uuids::uuid sourceEntityId, uuids::uuid targetEntityId, string content, Code code)
{
    this->id = (*uuidGenerator)();
    this->sourceEntityId = sourceEntityId;
    this->targetEntityId = targetEntityId;
    this->content = content;
    this->corrupted = false;
    this->code = code;
}

Message::~Message() {}

/* Getters */

uuids::uuid Message::getId() const
{
    return this->id;
}

uuids::uuid Message::getSourceEntityId() const
{
    return this->sourceEntityId;
}

uuids::uuid Message::getTargetEntityId() const
{
    return this->targetEntityId;
}

string Message::getContent() const
{
    return this->content;
}

bool Message::isCorrupted() const
{
    return this->corrupted;
}

Code Message::getCode() const
{
    return this->code;
}

/* Setters */

void Message::setCorrupted(bool isCorrupted)
{
    this->corrupted = isCorrupted;
}

/* Methods */

void Message::print(std::function<void(std::string)> printMessage) const
{
    printMessage("ID: " + uuids::to_string(this->getId()));
    printMessage("Source entity ID: " + uuids::to_string(this->getSourceEntityId()));
    printMessage("Target entity ID: " + uuids::to_string(this->getTargetEntityId()));
    printMessage("Code: " + codeToString(this->getCode()));
    printMessage("Corrupted: " + std::string(this->isCorrupted() ? "YES" : "NO"));
    printMessage("=== BEGIN ===");
    std::istringstream contentStream(this->getContent());
    std::string line;
    while (std::getline(contentStream, line))
    {
        printMessage(line);
    }
    printMessage("==== END ====");
}
