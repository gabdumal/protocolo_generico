#include <entity.hpp>

using namespace std;

/* Construction */

Entity::Entity(uuids::uuid_random_generator *uuidGenerator, string name)
{
    this->id = (*uuidGenerator)();
    this->setName(name);
    this->storage = "";
}

Entity::~Entity() {}

/* Getters */

uuids::uuid Entity::getId() const
{
    return this->id;
}

string Entity::getName() const
{
    return this->name;
}

/* Setters */

void Entity::setName(string name)
{
    this->name = name;
}

/* Methods */

void Entity::receiveMessage(const Message &message)
{
    this->storage += message.getContent() + "\n";
}

void Entity::printStorage(function<void(string)> printMessage) const
{
    printMessage("=== BEGIN ===");
    std::istringstream contentStream(this->storage);
    std::string line;
    while (std::getline(contentStream, line))
    {
        printMessage(line);
    }
    printMessage("==== END ====");
}