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

    optional<Message> receiveSynMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Message> receiveFinMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Message> receiveAckMessage(
        const Message &message,
        optional<uuids::uuid> &id_from_message_being_acknowledged,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Message> receiveAckSynMessage(
        const Message &message, uuids::uuid sent_message_id,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Message> receiveAckAckSynMessage(
        const Message &message, uuids::uuid sent_message_id,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Message> receiveNackMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    optional<Message> receiveDataMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);

    bool processMessageBeingSent(Message &message);

   public:
    struct Response {
        optional<Message> message;
        optional<uuids::uuid> id_from_message_possibly_acknowledged;
    };

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
    bool canSendMessage(uuids::uuid message_id) const;
    bool sendMessage(Message &message);
    void printMessageSendingInformation(Message &message,
                                        ostream &output_stream,
                                        PrettyConsole::Color color) const;
    Response receiveMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    void printStorage(function<void(string)> print_message) const;
    MessageConsequence getSendingMessageConsequence(
        const Message &message) const;
};

#endif  // _ENTITY_HPP
