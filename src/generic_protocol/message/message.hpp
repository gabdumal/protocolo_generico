#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <uuid.h>
#include <string>

using namespace std;

enum Code
{
    SYN,
    FIN,
    ACK,
    NACK,
    DATA
};

class Message
{
private:
    uuids::uuid id;
    uuids::uuid sourceEntityId;
    uuids::uuid targetEntityId;
    string content;
    bool corrupted;
    Code code;

public:
    /* Construction */
    Message(
        uuids::uuid_random_generator *uuidGenerator,
        uuids::uuid sourceEntityId,
        uuids::uuid targetEntityId,
        string content,
        Code code = Code::DATA);
    ~Message();

    /* Getters */
    uuids::uuid getId() const;
    uuids::uuid getSourceEntityId() const;
    uuids::uuid getTargetEntityId() const;
    string getContent() const;
    bool isCorrupted() const;
    Code getCode() const;

    /* Setters */
    void setCorrupted(bool isCorrupted);

    /* Methods */
    void print(function<void(string)> printMessage) const;
};

#endif // _MESSAGE_HPP
