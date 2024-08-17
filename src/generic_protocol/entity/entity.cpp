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
    setColor(TextColor::GREEN);
    cout << "Entity [" << this->getName() << "] " << endl;
    cout << TAB << "Message received: [" << message.getId() << "]" << endl;
    resetColor();
    return true;
}
