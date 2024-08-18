#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <uuid.h>
#include <string>

using namespace std;

enum Code
{
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

public:
    /* Construction */
    Message(
        uuids::uuid_random_generator *uuidGenerator,
        uuids::uuid sourceEntityId,
        uuids::uuid targetEntityId,
        string content);
    ~Message();

    /* Getters */
    uuids::uuid getId() const;
    uuids::uuid getSourceEntityId() const;
    uuids::uuid getTargetEntityId() const;
    string getContent() const;
    bool isCorrupted() const;

    /* Setters */
    void setCorrupted(bool isCorrupted);

    /* Methods */
    void print(std::function<void(std::string)> printMessage) const;
};

#endif // _MESSAGE_HPP