#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <uuid.h>
#include <console_colors.hpp>
#include <constants.hpp>
#include <message.hpp>
#include <string>
#include <unordered_map>

using namespace std;

class Entity
{
private:
    struct Connection
    {
        uuids::uuid synMessageId;
        uuids::uuid synAckMessageId;
    };

    uuids::uuid id;
    string name;
    string storage;
    unordered_map<uuids::uuid, Connection> connections;

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
    Message *receiveMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator);
    void printStorage(function<void(string)> printMessage) const;
    bool isConnectedTo(uuids::uuid entityId) const;
};

#endif // _ENTITY_HPP