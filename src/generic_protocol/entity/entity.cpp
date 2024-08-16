#include <entity.hpp>

using namespace std;

/* Construction */

Entity::Entity(string name)
{
    this->setName(name);
}

Entity::~Entity() {}

/* Getters */
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

void Entity::sendMessage(Message message)
{
    message.print();
}

void Entity::receiveMessage(Message message)
{
    message.print();
}