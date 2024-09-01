#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <uuid.h>

#include <functional>
#include <memory>
#include <message.hpp>
#include <optional>
#include <pretty_console.hpp>
#include <string>

using namespace std;

enum class ConnectionStep { NONE, SYN, ACK_SYN, ACK_ACK_SYN };

class Entity {
   public:
    struct Response {
        optional<Message> message;
        optional<uuids::uuid> id_from_message_possibly_acknowledged;

        Response(optional<Message> message,
                 optional<uuids::uuid> id_from_message_possibly_acknowledged)
            : message(message),
              id_from_message_possibly_acknowledged(
                  id_from_message_possibly_acknowledged) {}

        Response(optional<Message> message)
            : message(message),
              id_from_message_possibly_acknowledged(nullopt) {}

        Response()
            : message(nullopt),
              id_from_message_possibly_acknowledged(nullopt) {}
    };

   private:
    struct MessageConsequence {
        bool should_send_message;
        bool should_lock_entity;
    };

    uuids::uuid id;
    string name;
    string storage;

    function<void(uuids::uuid source_entity_id, uuids::uuid target_entity_id,
                  uuids::uuid message_id, ConnectionStep step)>
        connect_function;
    function<void(uuids::uuid source_entity_id, uuids::uuid target_entity_id)>
        remove_connection_function;
    function<bool(uuids::uuid source_entity_id, uuids::uuid target_entity_id,
                  ConnectionStep step)>
        is_connected_at_step_function;

    optional<Message> last_unacknowledged_message;

    /* Methods */

    void printInformation(
        string information, ostream &output_stream,
        PrettyConsole::Color color = PrettyConsole::Color::DEFAULT) const;

    Response receiveSynMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveFinMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveAckMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveAckSynMessage(
        const Message &message, uuids::uuid sent_message_id,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveAckAckSynMessage(
        const Message &message, uuids::uuid sent_message_id,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveAckAckAckSynMessage(
        const Message &message, uuids::uuid sent_message_id,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveNackMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveDataMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);

   public:
    /* Construction */
    Entity(uuids::uuid id, string name,
           function<void(uuids::uuid source_entity_id,
                         uuids::uuid target_entity_id, uuids::uuid message_id,
                         ConnectionStep step)>
               connect_function,
           function<void(uuids::uuid source_entity_id,
                         uuids::uuid target_entity_id)>
               remove_connection_function,
           function<bool(uuids::uuid source_entity_id,
                         uuids::uuid target_entity_id, ConnectionStep step)>
               is_connected_at_step_function)
        : id(id),
          name(name),
          connect_function(connect_function),
          remove_connection_function(remove_connection_function),
          is_connected_at_step_function(is_connected_at_step_function) {}

    ~Entity() {}

    /* Getters */
    uuids::uuid getId() const;
    string getName() const;

    /* Setters */
    void setName(string name);

    /* Methods */
    MessageConsequence getSendingMessageConsequence(
        const Message &message) const;
    bool canSendMessage(uuids::uuid message_id) const;

    bool sendMessage(Message &message);
    Response receiveMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);

    void printMessageInformation(const Message &message, ostream &output_stream,
                                 bool is_sending) const;
    void printStorage(function<void(string)> print_message) const;

    void connect(uuids::uuid target_entity_id, uuids::uuid message_id,
                 ConnectionStep step);
    void removeConnection(uuids::uuid target_entity_id);
    bool isConnectedAtStep(uuids::uuid target_entity_id,
                           ConnectionStep step) const;
};

#endif  // _ENTITY_HPP
