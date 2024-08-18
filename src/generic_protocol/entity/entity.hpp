#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <message.hpp>
#include <uuid.h>
#include <constants.hpp>
#include <console_colors.hpp>
#include <string>

using namespace std;

class Entity
{
private:
    uuids::uuid id;
    string name;
    string storage;

public:
    /* Construction */
    Entity(uuids::uuid_random_generator *uuidGenerator, string name);
    ~Entity();

    /* Getters */
    uuids::uuid getId() const;
    string getName() const;

    /* Setters */
    void setName(string name);

    /* Methods */
    void receiveMessage(const Message &message);
    void printStorage(function<void(string)> printMessage) const;
};

#endif // _ENTITY_HPP