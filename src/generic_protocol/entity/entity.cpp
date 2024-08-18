#include <entity.hpp>

using namespace std;

/* Construction */

Entity::Entity(uuids::uuid_random_generator *uuidGenerator, string name)
{
    this->id = (*uuidGenerator)();
    this->setName(name);
}

Entity::~Entity() {}

/* Getters */

uuids::uuid Entity::getId()
{
    return this->id;
}

string Entity::getName()
{
    return this->name;
}

/* Setters */

void Entity::setName(string name)
{
    this->name = name;
}

/* Methods */

bool Entity::receiveMessage(Message message)
{
    ostringstream outputStream;
    setColor(outputStream, TextColor::CYAN);
    outputStream << "Entity [" << this->getName() << "] " << endl;
    outputStream << TAB << "Message received: [" << message.getId() << "]" << endl;
    outputStream << TAB << "Corrupted: " << (message.isCorrupted() ? "TRUE" : "FALSE") << endl;
    resetColor(outputStream);
    cout << outputStream.str();
    return true;
}
