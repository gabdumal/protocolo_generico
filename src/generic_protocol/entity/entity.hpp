#ifndef _ENTITY_H
#define _ENTITY_H

#include <string>

using namespace std;

class Entity
{
private:
    string name;

public:
    // Construction
    Entity(string name);
    ~Entity();

    // Getters
    string getName();

    // Setters
    void setName(string name);

    // Other methods
    void sendMessage(string Message);
};

#endif // _ENTITY_H