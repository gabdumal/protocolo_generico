#include "entity.hpp"
#include <iostream>

using namespace std;

// Construction

Entity::Entity(string name)
{
    this->setName(name);
}

Entity::~Entity() {}

// Getters

string Entity::getName()
{
    return this->name;
}

// Setters

void Entity::setName(string name)
{
    this->name = name;
}

// Other methods

void Entity::sendMessage(string message)
{
    cout << message << endl;
}