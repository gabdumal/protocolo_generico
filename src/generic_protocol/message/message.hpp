#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <uuid.h>

#include <memory>
#include <string>

using namespace std;

class Message {
   public:
    enum class Code { SYN, FIN, ACK, NACK, DATA };
    enum class CodeVariant {
        ACK,
        ACK_SYN,
        ACK_ACK_SYN,
        ACK_ACK_ACK_SYN,
        ACK_FIN,
        ACK_ACK_FIN,
        NACK,
        NACK_SYN,
        NACK_ACK_SYN,
        NACK_ACK_ACK_SYN,
        NACK_FIN,
    };

    static string codeToString(Code code);

   private:
    uuids::uuid id;
    uuids::uuid source_entity_id;
    uuids::uuid target_entity_id;
    Code code;
    optional<CodeVariant> code_variant;
    optional<uuids::uuid> id_from_message_being_acknowledged;
    string content;

   public:
    /* Construction */
    Message(shared_ptr<uuids::uuid_random_generator> uuid_generator,
            uuids::uuid source_entity_id, uuids::uuid target_entity_id,
            Code code, optional<CodeVariant> code_variant,
            optional<uuids::uuid> id_from_message_being_acknowledged,
            string content);

    Message(shared_ptr<uuids::uuid_random_generator> uuid_generator,
            uuids::uuid source_entity_id, uuids::uuid target_entity_id,
            Code code, optional<CodeVariant> code_variant,
            optional<uuids::uuid> id_from_message_being_acknowledged)
        : Message(uuid_generator, source_entity_id, target_entity_id, code,
                  code_variant, id_from_message_being_acknowledged, "") {}

    ~Message();

    /* Getters */
    uuids::uuid getId() const;
    uuids::uuid getSourceEntityId() const;
    uuids::uuid getTargetEntityId() const;
    Code getCode() const;
    optional<CodeVariant> getCodeVariant() const;
    optional<uuids::uuid> getIdFromMessageBeingAcknowledged() const;
    string getContent() const;

    /* Setters */
    void setCodeVariant(CodeVariant code_variant);
    void setIdFromMessageBeingAcknowledged(uuids::uuid id_from_message);

    /* Methods */
    void print(function<void(string)> print_message) const;
};

#endif  // _MESSAGE_HPP
