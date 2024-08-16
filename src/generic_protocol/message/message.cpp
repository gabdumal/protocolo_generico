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

uuids::uuid Message::getId()
{
    return this->id;
}

uuids::uuid Message::getSourceEntityId()
{
    return this->sourceEntityId;
}

uuids::uuid Message::getTargetEntityId()
{
    return this->targetEntityId;
}

string Message::getContent()
{
    return this->content;
}

/* Methods */

void Message::print(std::function<void(std::string)> printMessage)
{
    printMessage("ID: " + uuids::to_string(this->getId()));
    printMessage("Source entity ID: " + uuids::to_string(this->getSourceEntityId()));
    printMessage("Target entity ID: " + uuids::to_string(this->getTargetEntityId()));
    printMessage("=== BEGIN ===");
    printMessage(this->getContent());
    printMessage("=== END ===");
}
