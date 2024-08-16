#ifndef _ENTITY_H
#define _ENTITY_H

#include <message.hpp>
#include <string>

using namespace std;

class Entity
{
private:
    string name;

public:
    /* Construction */
    Entity(string name);
    ~Entity();

    /* Getters */
    string getName();

    /* Setters */
    void setName(string name);

    /* Methods */
    void sendMessage(Message message);
    void receiveMessage(Message message);
};

#endif // _ENTITY_H