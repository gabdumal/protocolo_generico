#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <message.hpp>
#include <uuid.h>
#include <constants.hpp>
#include <console_colors.hpp>
#include <string>
#include <iostream>

using namespace std;

class Entity
{
private:
    uuids::uuid id;
    string name;

public:
    /* Construction */
    Entity(uuids::uuid_random_generator *uuidGenerator, string name);
    ~Entity();

    /* Getters */
    uuids::uuid getId();
    string getName();

    /* Setters */
    void setName(string name);

    /* Methods */
    bool receiveMessage(Message message);
};

#endif // _ENTITY_HPP