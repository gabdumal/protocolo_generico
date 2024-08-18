#include <entity.hpp>
#include <generic_protocol_constants.hpp>

using namespace std;

/* Auxiliary */

string getLineContent(int line, string content)
{
    istringstream contentStream(content);
    string lineContent;
    for (int i = 0; i < line; i++)
    {
        getline(contentStream, lineContent);
    }
    return lineContent;
}

/* Construction */

Entity::Entity(uuids::uuid_random_generator *uuidGenerator, string name)
{
    this->id = (*uuidGenerator)();
    this->setName(name);
    this->storage = "";
    this->connections = unordered_map<uuids::uuid, shared_ptr<Connection>>();
    this->lastUnacknowledgedMessageId = nullopt;
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

optional<Message> Entity::receiveMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    if (GenericProtocolConstants::debugInformation)
    {
        ostringstream messageContent;
        message.print([&messageContent](string line)
                      { messageContent << ConsoleColors::tab << line << endl; });
        this->printInformation("Received message\n" + messageContent.str(), cout);
    }

    switch (message.getCode())
    {
    case Code::SYN:
        return this->receiveSynMessage(message, uuidGenerator);

    case Code::FIN:
        return this->receiveFinMessage(message, uuidGenerator);

    case Code::ACK:
        return this->receiveAckMessage(message, uuidGenerator);

    case Code::NACK:
        return this->receiveNackMessage(message, uuidGenerator);

    case Code::DATA:
        return this->receiveDataMessage(message, uuidGenerator);
    }

    return nullopt;
}

void Entity::printStorage(function<void(string)> printMessage) const
{
    printMessage("=== BEGIN ===");
    istringstream contentStream(this->storage);
    string line;
    while (getline(contentStream, line))
    {
        printMessage(line);
    }
    printMessage("==== END ====");
}

bool Entity::isConnectedTo(uuids::uuid entityId) const
{
    return this->connections.find(entityId) != this->connections.end();
}

bool Entity::canReceiveDataFrom(uuids::uuid entityId) const
{
    auto connection = this->connections.find(entityId);
    return connection != this->connections.end() && connection->second->ackAckSynMessageId.has_value();
}

optional<Message> Entity::receiveSynMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    if (!this->isConnectedTo(message.getSourceEntityId()))
    {
        Message ackSynMessage(uuidGenerator, this->id, message.getSourceEntityId(), "ACK-SYN\n" + to_string(message.getId()), Code::ACK);

        // Still need to receive the ACK-ACK-SYN message
        auto connection = make_shared<Connection>(
            Connection{
                message.getId(),
                ackSynMessage.getId(),
                nullopt});

        this->connections.insert(pair<uuids::uuid, shared_ptr<Connection>>(message.getSourceEntityId(), connection));
        return ackSynMessage;
    }
    else
    {
        return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK-SYN\n" + to_string(message.getId()), Code::NACK);
    }
}

optional<Message> Entity::receiveFinMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    if (this->isConnectedTo(message.getSourceEntityId()))
        this->connections.erase(message.getSourceEntityId());
    return nullopt;
}

optional<Message> Entity::receiveAckMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    string firstLine = getLineContent(1, message.getContent());
    if (firstLine == "ACK-SYN")
        return this->receiveAckSynMessage(message, uuidGenerator);
    else if (firstLine == "ACK-ACK-SYN")
        return this->receiveAckAckSynMessage(message, uuidGenerator);
    return nullopt;
}

optional<Message> Entity::receiveAckSynMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    string secondLine = getLineContent(2, message.getContent());
    optional<uuids::uuid> uuidContainer = uuids::uuid::from_string(secondLine);

    if (uuidContainer.has_value())
    {
        uuids::uuid synMessageId = uuidContainer.value();
        Message ackAckSynMessage(uuidGenerator, this->id, message.getSourceEntityId(), "ACK-ACK-SYN\n" + to_string(message.getId()), Code::ACK);

        auto connection = make_shared<Connection>(
            Connection{
                synMessageId,
                message.getId(),
                ackAckSynMessage.getId(),
            });

        this->connections.insert(pair<uuids::uuid, shared_ptr<Connection>>(message.getSourceEntityId(), connection));

        return ackAckSynMessage;
    }

    return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
}

optional<Message> Entity::receiveAckAckSynMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    string secondLine = getLineContent(2, message.getContent());
    optional<uuids::uuid> uuidContainer = uuids::uuid::from_string(secondLine);

    if (uuidContainer.has_value())
    {
        uuids::uuid ackSynMessageId = uuidContainer.value();

        auto connection = this->connections.find(message.getSourceEntityId());

        if (this->isConnectedTo(message.getSourceEntityId()) && connection->second->ackSynMessageId == ackSynMessageId)
        {
            // Update connection
            connection->second->ackAckSynMessageId = message.getId();
            return nullopt;
        }
    }

    return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK-ACK-ACK-SYN \n" + to_string(message.getId()), Code::NACK);
}

optional<Message> Entity::receiveNackMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    return nullopt;
}

optional<Message> Entity::receiveDataMessage(const Message &message, uuids::uuid_random_generator *uuidGenerator)
{
    if (this->isConnectedTo(message.getSourceEntityId()))
    {
        this->storage += message.getContent() + "\n";

        return Message(uuidGenerator, this->id, message.getSourceEntityId(), "ACK\n" + to_string(message.getId()), Code::ACK);
    }

    return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
}

bool Entity::sendMessage(Message &message)
{
    if (!canSendMessage())
        return false;

    if (isConnectedTo(message.getTargetEntityId()))
    {
        lastUnacknowledgedMessageId = message.getId();
        return true;
    }
    else if (message.getCode() == Code::SYN)
    {
        lastUnacknowledgedMessageId = message.getId();
        return true;
    }

    return false;
}

bool Entity::canSendMessage() const
{
    // If there is no unacknowledged message, the entity can send a new message
    return !this->lastUnacknowledgedMessageId.has_value();
}

void Entity::printInformation(
    string information,
    ostream &outputStream,
    ConsoleColors::Color color) const
{
    string header = "Entity " + this->getName() + " [" + to_string(this->getId()) + "]";
    ConsoleColors::printInformation(header, information, outputStream, ConsoleColors::Color::BRIGHT_WHITE, ConsoleColors::Color::CYAN, color);
}