#include <message.hpp>
#include <iostream>

using namespace std;

/* Construction */

Message::Message(uuids::uuid id, string content)
{
    this->id = id;
    this->content = content;
}

Message::~Message() {}

/* Getters */

uuids::uuid Message::getId()
{
    return this->id;
}

string Message::getContent()
{
    return this->content;
}

/* Methods */

void Message::print()
{
    cout << "Message" << endl;
    cout << "ID: " << this->getId() << endl;
    cout << "=== BEGIN ===" << endl;
    cout << this->getContent();
    cout << "\n=== END ===" << endl
         << endl;
}
