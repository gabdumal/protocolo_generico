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
    outputStream << "Entity [" << this->id << "] received message from entity [" << message.getSourceEntityId() << "] with ID [" + to_string(message.getId()) + "] and code " << message.getCode() << " and content: " << message.getContent();
    resetColor(outputStream);
    cout << outputStream.str() << endl;

    switch (message.getCode())
    {
    case Code::SYN:
    {
        if (!this->isConnectedTo(message.getSourceEntityId()))
        {
            Message *synAckMessage = new Message(uuidGenerator, this->id, message.getSourceEntityId(), "SYN-ACK\n" + to_string(message.getId()), Code::ACK);
            this->connections.insert(pair<uuids::uuid, Connection>(message.getSourceEntityId(), Connection{
                                                                                                    message.getId(),
                                                                                                    synAckMessage->getId(),
                                                                                                }));
            return synAckMessage;
        }
        else
        {
            return new Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
        }
    }

    case Code::FIN:
    {
        if (this->isConnectedTo(message.getSourceEntityId()))
            this->connections.erase(message.getSourceEntityId());
        break;
    }

    case Code::ACK:
    {
        string content = message.getContent();
        string firstLine = content.substr(0, content.find("\n"));
        if (firstLine == "SYN-ACK")
        { // If it is a SYN-ACK message, try to extract the SYN message ID
            string secondLine = content.substr(content.find("\n") + 1);
            std::optional<uuids::uuid> possibleUuid = uuids::uuid::from_string(secondLine);
            if (possibleUuid.has_value())
            {
                uuids::uuid synMessageId = possibleUuid.value();
                Message *synAckAckMessage = new Message(uuidGenerator, this->id, message.getSourceEntityId(), "SYN-ACK-ACK\n" + to_string(message.getId()), Code::ACK);

                this->connections.insert(pair<uuids::uuid, Connection>(message.getSourceEntityId(), Connection{
                                                                                                        synMessageId,
                                                                                                        message.getId(),
                                                                                                        synAckAckMessage->getId(),
                                                                                                    }));
                return synAckAckMessage;
            }
            else
            {
                return new Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
            }
        }
        else if (firstLine == "SYN-ACK-ACK")
        { // If it is a SYN-ACK-ACK message, try to extract the SYN-ACK message ID
            string secondLine = content.substr(content.find("\n") + 1);
            std::optional<uuids::uuid> possibleUuid = uuids::uuid::from_string(secondLine);
            if (possibleUuid.has_value())
            {
                uuids::uuid synAckMessageId = possibleUuid.value();
                auto connection = this->connections.find(message.getSourceEntityId());
                if (connection != this->connections.end() && connection->second.synAckMessageId == synAckMessageId)
                {
                    connection->second.synAckAckMessageId = message.getId();
                }
                else
                {
                    return new Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
                }
            }
            else
            {
                return new Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
            }
        }
        break;
    }

    case Code::NACK:
    {
        break;
    }

    case Code::DATA:
    {
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