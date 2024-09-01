#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <uuid.h>

#include <memory>
#include <message.hpp>
#include <optional>
#include <pretty_console.hpp>
#include <string>
#include <unordered_map>

using namespace std;

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
    struct Connection {
        optional<uuids::uuid> syn_message_id;
        optional<uuids::uuid> ack_syn_message_id;
        optional<uuids::uuid> ack_ack_syn_message_id;
    };

    struct MessageConsequence {
        bool should_send_message;
        bool should_lock_entity;
    };

    uuids::uuid id;
    string name;
    string storage;

    unordered_map<uuids::uuid, shared_ptr<Connection>> connections;
    optional<Message> last_unacknowledged_message;

    /* Methods */
    void printInformation(
        string information, ostream &output_stream,
        PrettyConsole::Color color = PrettyConsole::Color::DEFAULT) const;
    optional<Connection> getConnection(uuids::uuid entity_id) const;
    bool isConnectedTo(uuids::uuid entity_id) const;
    bool canReceiveDataFrom(uuids::uuid entity_id) const;

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
    Entity(string name,
           shared_ptr<uuids::uuid_random_generator> uuid_generator);
    ~Entity();

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
};

#endif  // _ENTITY_HPP
