#ifndef _GENERIC_PROTOCOL_HPP
#define _GENERIC_PROTOCOL_HPP

#include <message.hpp>
#include <entity.hpp>
#include <network.hpp>
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
    static Entity createEntity(string name, ostringstream &outputStream);
    static void sendMessage(Entity source, Entity target, string messageContent, Network &network, ostringstream &outputStream);
};

#endif // _GENERIC_PROTOCOL_HPP
