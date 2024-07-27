#ifndef _GENERIC_PROTOCOL_H
#define _GENERIC_PROTOCOL_H

class GenericProtocol
{
public:
    static void run();
    static Entity createEntity(string name);
    static void sendMessage(Entity entity, string message);
};

#endif // _GENERIC_PROTOCOL_H