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

public:
    /* Construction */
    Message(
        uuids::uuid_random_generator *uuidGenerator,
        uuids::uuid sourceEntityId,
        uuids::uuid targetEntityId,
        string content);
    ~Message();

    /* Getters */
    uuids::uuid getId();
    uuids::uuid getSourceEntityId();
    uuids::uuid getTargetEntityId();
    string getContent();

    /* Methods */
    void print(std::function<void(std::string)> printMessage);
};

#endif // _MESSAGE_HPP