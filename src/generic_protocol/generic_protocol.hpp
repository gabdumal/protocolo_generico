#ifndef _GENERIC_PROTOCOL_HPP
#define _GENERIC_PROTOCOL_HPP

#include <message.hpp>
#include <entity.hpp>
#include <network.hpp>
#include <uuid.h>
#include <memory>

class GenericProtocol
{
private:
    static shared_ptr<uuids::uuid_random_generator> uuidGenerator;

public:
    /* Construction */
    GenericProtocol(shared_ptr<uuids::uuid_random_generator> uuidGenerator);
    ~GenericProtocol();

    /* Methods */
    void run();

    /* Static methods */
    static shared_ptr<Entity> createEntity(string name, function<void(string)> printMessage);
    static void sendMessage(shared_ptr<Entity> source, shared_ptr<Entity> target, string messageContent, Code messageCode, Network &network, ostringstream &outputStream);
};

#endif // _GENERIC_PROTOCOL_HPP
