#include <message.hpp>
#include <iostream>

using namespace std;

/* Construction */

Message::Message(uuids::uuid_random_generator *uuidGenerator, uuids::uuid sourceEntityId, uuids::uuid targetEntityId, string content)
{
    this->id = (*uuidGenerator)();
    this->sourceEntityId = sourceEntityId;
    this->targetEntityId = targetEntityId;
    this->content = content;
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

/* Methods */

void Message::print(std::function<void(std::string)> printMessage) const
{
    printMessage("ID: " + uuids::to_string(this->getId()));
    printMessage("Source entity ID: " + uuids::to_string(this->getSourceEntityId()));
    printMessage("Target entity ID: " + uuids::to_string(this->getTargetEntityId()));
    printMessage("=== BEGIN ===");
    printMessage(this->getContent());
    printMessage("=== END ===");
}
