#ifndef _GENERIC_PROTOCOL_HPP
#define _GENERIC_PROTOCOL_HPP

#include <uuid.h>

#include <list>
#include <memory>

#include "./entity/entity.hpp"
#include "./message/message.hpp"
#include "./network/network.hpp"
#include "connection.hpp"

class GenericProtocol {
   private:
    static shared_ptr<uuids::uuid_random_generator> uuid_generator;

    static void printSendingMessageHeader(shared_ptr<Entity> source,
                                          shared_ptr<Entity> target,
                                          string message_content,
                                          Message::Code message_code,
                                          Network &network,
                                          ostringstream &output_stream);
    static void printSendingMessageFooter(bool has_been_processed,
                                          ostringstream &output_stream);

   public:
    /* Construction */
    GenericProtocol(shared_ptr<uuids::uuid_random_generator> uuid_generator);
    ~GenericProtocol();

    /* Methods */
    void run();

    /* Static methods */
    static shared_ptr<Entity> createEntity(
        string name, list<shared_ptr<Entity>> &entities,
        map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
        function<void(string)> print_message);

    static void sendMessage(shared_ptr<Entity> source,
                            shared_ptr<Entity> target, string message_content,
                            Message::Code message_code, Network &network,
                            ostringstream &output_stream);
};

#endif  // _GENERIC_PROTOCOL_HPP
