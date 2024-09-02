#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <uuid.h>

#include <memory>
#include <string>

using namespace std;

class Message {
   public:
    enum class Code { SYN, FIN, ACK, NACK, DATA };
    enum class AckType { ACK, ACK_SYN, ACK_ACK_SYN };

    static string codeToString(Code code);

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
    ~Message();

    /* Getters */
    uuids::uuid getId() const;
    uuids::uuid getSourceEntityId() const;
    uuids::uuid getTargetEntityId() const;
    string getContent() const;
    bool isCorrupted() const;
    Code getCode() const;
    optional<uuids::uuid> getIdFromMessageBeingAcknowledged() const;
    optional<AckType> getAckType() const;

    /* Setters */
    void setCorrupted(bool is_corrupted);

    /* Methods */
    void print(function<void(string)> print_message) const;
};

struct Package {
    Message message;
    bool should_be_confirmed;

    Package(Message message, bool should_be_confirmed)
        : message(message), should_be_confirmed(should_be_confirmed) {}
};

#endif  // _MESSAGE_HPP
