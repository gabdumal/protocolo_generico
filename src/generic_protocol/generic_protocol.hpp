#ifndef _GENERIC_PROTOCOL_H
#define _GENERIC_PROTOCOL_H

#include <entity.hpp>
#include <message.hpp>
#include <uuid.h>

class GenericProtocol
{
private:
    static uuids::uuid_random_generator *uuidGenerator;

public:
    /* Construction */
    GenericProtocol(uuids::uuid_random_generator *uuidGenerator);
    ~GenericProtocol();

    /* Methods */
    void run();

    /* Static methods */
    static Entity createEntity(string name);
    static void sendMessage(Entity entity, Message message);
};

#endif // _GENERIC_PROTOCOL_H