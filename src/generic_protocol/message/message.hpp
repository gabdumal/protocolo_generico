#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <uuid.h>

#include <memory>
#include <string>

using namespace std;

class Message {
   public:
    enum Code { SYN, FIN, ACK, NACK, DATA };

   private:
    uuids::uuid id;
    uuids::uuid source_entity_id;
    uuids::uuid target_entity_id;
    string content;
    bool corrupted;
    Code code;

   public:
    /* Construction */
    Message(shared_ptr<uuids::uuid_random_generator> uuid_generator,
            uuids::uuid source_entity_id, uuids::uuid target_entity_id,
            string content, Code code = Code::DATA);
    // Message(const Message& message)
    //     : id(message.id),
    //       source_entity_id(message.source_entity_id),
    //       target_entity_id(message.target_entity_id),
    //       content(message.content),
    //       corrupted(message.corrupted),
    //       code(message.code) {}
    ~Message();

    /* Getters */
    uuids::uuid getId() const;
    uuids::uuid getSourceEntityId() const;
    uuids::uuid getTargetEntityId() const;
    string getContent() const;
    bool isCorrupted() const;
    Code getCode() const;
    optional<uuids::uuid> getIdFromMessageBeingAcknowledged() const;

    /* Setters */
    void setCorrupted(bool is_corrupted);

    /* Methods */
    void print(function<void(string)> print_message) const;
};

#endif  // _MESSAGE_HPP
