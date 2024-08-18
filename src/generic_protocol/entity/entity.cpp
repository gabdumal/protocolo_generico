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
    // message.print([](string message)
    //               { cout << message << endl; });
    ostringstream outputStream;
    setColor(outputStream, TextColor::GREEN);
    outputStream << "Entity [" << this->getName() << "] " << endl;
    outputStream << TAB << "Message received: [" << message.getId() << "]" << endl;
    resetColor(outputStream);
    cout << outputStream.str();
    return true;
}
