#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <uuid.h>
#include <string>

using namespace std;

class Message
{
private:
    // uuids::uuid origin;
    // uuids::uuid target;
    uuids::uuid id;
    string content;

public:
    /* Construction */
    Message(
        // uuids::uuid origin, uuids::uuid target,
        uuids::uuid id,
        string content);
    ~Message();

    /* Getters */
    uuids::uuid getId();
    string getContent();

    /* Methods */
    void print();
};

#endif // _MESSAGE_H