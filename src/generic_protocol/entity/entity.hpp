#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include <uuid.h>
#include <console_colors.hpp>
#include <constants.hpp>
#include <message.hpp>
#include <string>
#include <unordered_map>
#include <optional>
#include <memory>
#include <chrono>
#include <thread>

using namespace std;

class Entity
{
private:
    struct Connection
    {
        optional<uuids::uuid> synMessageId;
        optional<uuids::uuid> ackSynMessageId;
        optional<uuids::uuid> ackAckSynMessageId;
    };

    uuids::uuid id;
    string name;
    string storage;
    unordered_map<uuids::uuid, shared_ptr<Connection>> connections;
    optional<uuids::uuid> lastUnacknowledgedMessageId;

    /* Methods */
    void printInformation(string information, ostream &outputStream, ConsoleColors::Color color = ConsoleColors::Color::DEFAULT) const;
    bool isConnectedTo(uuids::uuid entityId) const;
    bool canReceiveDataFrom(uuids::uuid entityId) const;
    optional<Message> receiveSynMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator);
    optional<Message> receiveFinMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator);
    optional<Message> receiveAckMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator);
    optional<Message> receiveAckSynMessage(const Message &message, uuids::uuid sentMessageId, uuids::uuid_random_generator *uuidGenerator);
    optional<Message> receiveAckAckSynMessage(const Message &message, uuids::uuid sentMessageId, uuids::uuid_random_generator *uuidGenerator);
    optional<Message> receiveNackMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator);
    optional<Message> receiveDataMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator);

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
    bool canSendMessage() const;
    bool sendMessage(Message &message);
    optional<Message> receiveMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator);
    void printStorage(function<void(string)> printMessage) const;
};

#endif // _ENTITY_HPP