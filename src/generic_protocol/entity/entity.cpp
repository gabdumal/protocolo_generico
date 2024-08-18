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

Message *Entity::receiveMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    ostringstream outputStream;
    setColor(outputStream, Color::CYAN);
    outputStream << "Entity " << this->id << " received message from entity " << message.getSourceEntityId() << " with code " << message.getCode() << " and content: " << message.getContent();
    resetColor(outputStream);
    cout << outputStream.str() << endl;

    switch (message.getCode())
    {
    case Code::SYN:
        if (!this->isConnectedTo(message.getSourceEntityId()))
        {
            Message *synAckMessage = new Message(uuidGenerator, this->id, message.getSourceEntityId(), "SYN-ACK", Code::ACK);
            this->connections.insert(pair<uuids::uuid, Connection>(message.getSourceEntityId(), Connection{
                                                                                                    message.getId(),
                                                                                                    synAckMessage->getId(),
                                                                                                }));
            return synAckMessage;
        }
        break;

    case Code::FIN:
        if (this->isConnectedTo(message.getSourceEntityId()))
            this->connections.erase(message.getSourceEntityId());
        break;

    case Code::ACK:
        if (this->isConnectedTo(message.getSourceEntityId()))
        {
            if (message.getContent() == "SYN-ACK")
            {
                return new Message(uuidGenerator, this->id, message.getSourceEntityId(), "SYN-ACK-ACK", Code::ACK);
            }
        }

        break;

    case Code::NACK:

        break;

    case Code::DATA:
        if (this->isConnectedTo(message.getSourceEntityId()))
        {
            this->storage += message.getContent() + "\n";
            return new Message(uuidGenerator, this->id, message.getSourceEntityId(), "ACK\n" + to_string(message.getId()), Code::ACK);
        }
        else
        {
            return new Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
        }
    }

    return nullptr;
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

bool Entity::isConnectedTo(uuids::uuid entityId) const
{
    return this->connections.find(entityId) != this->connections.end();
}