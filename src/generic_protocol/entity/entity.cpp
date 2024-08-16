#include <entity.hpp>
#include <iostream>

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

void Entity::receiveMessage(Message message)
{
    message.print([](string message)
                  { cout << message << endl; });
}
