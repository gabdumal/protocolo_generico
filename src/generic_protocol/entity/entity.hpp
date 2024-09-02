#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <uuid.h>

#include <functional>
#include <list>
#include <memory>
#include <message.hpp>
#include <pretty_console.hpp>

using namespace std;

enum class ConnectionStep { NONE, SYN, ACK_SYN, ACK_ACK_SYN };

using InternalConnectFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, ConnectionStep>;
using ConnectFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, uuids::uuid, ConnectionStep>;
using ConnectFunction = shared_ptr<
    function<void(ConnectFunctionParameters connect_function_parameters)>>;

using InternalRemoveConnectionFunctionParameters = tuple<uuids::uuid>;
using RemoveConnectionFunctionParameters = tuple<uuids::uuid, uuids::uuid>;
using RemoveConnectionFunction = shared_ptr<function<void(
    RemoveConnectionFunctionParameters remove_connection_function_parameters)>>;

using InternalIsConnectedAtStepFunctionParameters =
    tuple<uuids::uuid, ConnectionStep>;
using IsConnectedAtStepFunctionParameters =
    tuple<uuids::uuid, uuids::uuid, ConnectionStep>;
using IsConnectedAtStepFunction =
    shared_ptr<function<bool(IsConnectedAtStepFunctionParameters
                                 is_connected_at_step_function_parameters)>>;

class Entity {
   public:
    struct Response {
        optional<Message> message;
        bool should_be_confirmed;
        optional<uuids::uuid> id_from_message_possibly_acknowledged;

        Response(optional<Message> message, bool should_be_confirmed,
                 optional<uuids::uuid> id_from_message_possibly_acknowledged)
            : message(message),
              should_be_confirmed(should_be_confirmed),
              id_from_message_possibly_acknowledged(
                  id_from_message_possibly_acknowledged) {}
    };

   private:
    struct MessageConsequence {
        bool should_send_message;
        bool should_lock_entity;
    };

    uuids::uuid id;
    string name;
    string storage;

    ConnectFunction connect_function;
    RemoveConnectionFunction remove_connection_function;
    IsConnectedAtStepFunction is_connected_at_step_function;

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
    Response receiveNackMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);
    Response receiveDataMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);

   public:
    /* Construction */
    Entity(uuids::uuid id, string name, ConnectFunction connect_function,
           RemoveConnectionFunction remove_connection_function,
           IsConnectedAtStepFunction is_connected_at_step_function)
        : id(id),
          name(name),
          storage(""),
          connect_function(connect_function),
          remove_connection_function(remove_connection_function),
          is_connected_at_step_function(is_connected_at_step_function),
          last_unacknowledged_message(nullopt) {}

    ~Entity() {}

    /* Getters */
    uuids::uuid getId() const;
    string getName() const;

    /* Setters */
    void setName(string name);

    /* Methods */
    bool canSendMessage(uuids::uuid message_id) const;
    // MessageConsequence getSendingMessageConsequence(
    //     const Message &message) const;
    // bool shouldBeConfirmed(Message &message) const;

    bool sendMessage(Message message, bool should_be_confirmed);
    Response receiveMessage(
        const Message &message,
        shared_ptr<uuids::uuid_random_generator> uuid_generator);

    void printMessageInformation(const Message &message, ostream &output_stream,
                                 bool is_sending) const;
    void printStorage(function<void(string)> print_message) const;

    /* Connection */
    void connect(InternalConnectFunctionParameters connect_function_parameters);
    void removeConnection(InternalRemoveConnectionFunctionParameters
                              remove_connection_function_parameters);
    bool isConnectedAtStep(InternalIsConnectedAtStepFunctionParameters
                               is_connected_at_step_function_parameters) const;
};

using EntitiesList = list<shared_ptr<Entity>>;

#endif  // _ENTITY_HPP
