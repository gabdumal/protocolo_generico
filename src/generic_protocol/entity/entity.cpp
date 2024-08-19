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

Entity::Entity(string name, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    this->id = (*uuidGenerator)();
    this->setName(name);
    this->storage = "";
    this->connections = unordered_map<uuids::uuid, shared_ptr<Connection>>();
    this->lastUnacknowledgedMessage = nullopt;
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

void Entity::printInformation(
    string information,
    ostream &outputStream,
    ConsoleColors::Color color) const
{
    string header = "Entity " + this->getName() + " [" + to_string(this->getId()) + "]";
    ConsoleColors::printInformation(header, information, outputStream, ConsoleColors::Color::BRIGHT_WHITE, ConsoleColors::Color::CYAN, color);
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

bool Entity::canSendMessage() const
{
    // If there is no unacknowledged message, the entity can send a new message
    return !this->lastUnacknowledgedMessage.has_value();
}

bool Entity::canReceiveDataFrom(uuids::uuid entityId) const
{
    auto connection = this->connections.find(entityId);
    return connection != this->connections.end() && connection->second->ackAckSynMessageId.has_value();
}

bool Entity::sendMessage(Message &message)
{
    if (GenericProtocolConstants::debugInformation)
    {
        ostringstream messageContent;
        message.print([&messageContent](string line)
                      { messageContent << ConsoleColors::tab << line << endl; });
        optional<uuids::uuid> lastUnacknowledgedMessageId = this->lastUnacknowledgedMessage.has_value() ? optional<uuids::uuid>(this->lastUnacknowledgedMessage.value().getId()) : nullopt;
        string lastUnacknowledgedMessageIdAsString = lastUnacknowledgedMessageId.has_value() ? uuids::to_string(lastUnacknowledgedMessageId.value()) : "N/A";
        string information = "Last unacknowledged message ID: [" + lastUnacknowledgedMessageIdAsString + "]\nTrying to send message\n" + messageContent.str();
        this->printInformation(information, cout, ConsoleColors::Color::YELLOW);
    }

    if (!canSendMessage())
        return false;

    if (isConnectedTo(message.getTargetEntityId()))
    {
        this->lastUnacknowledgedMessage->getId() = message.getId();
        return true;
    }
    else if (message.getCode() == Code::SYN)
    {
        this->lastUnacknowledgedMessage->getId() = message.getId();
        return true;
    }

    return false;
}

optional<Message> Entity::receiveMessage(const Message &message, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    if (GenericProtocolConstants::debugInformation)
    {
        ostringstream messageContent;
        message.print([&messageContent](string line)
                      { messageContent << ConsoleColors::tab << line << endl; });
        optional<uuids::uuid> lastUnacknowledgedMessageId = this->lastUnacknowledgedMessage.has_value() ? optional<uuids::uuid>(this->lastUnacknowledgedMessage.value().getId()) : nullopt;
        string lastUnacknowledgedMessageIdAsString = lastUnacknowledgedMessageId.has_value() ? uuids::to_string(lastUnacknowledgedMessageId.value()) : "N/A";
        string information = "Last unacknowledged message ID: [" + lastUnacknowledgedMessageIdAsString + "]\nTrying to send message\n" + messageContent.str();
        this->printInformation(information, cout, ConsoleColors::Color::MAGENTA);
    }

    if (message.isCorrupted())
    {
        return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
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

optional<Message> Entity::receiveSynMessage(const Message &message, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
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

optional<Message> Entity::receiveFinMessage(const Message &message, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    if (this->isConnectedTo(message.getSourceEntityId()))
        this->connections.erase(message.getSourceEntityId());
    return nullopt;
}

optional<Message> Entity::receiveAckMessage(const Message &message, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    string firstLine = getLineContent(1, message.getContent());
    string secondLine = getLineContent(2, message.getContent());

    optional<uuids::uuid> uuidContainer = uuids::uuid::from_string(secondLine);
    if (uuidContainer.has_value())
    {
        uuids::uuid sentMessageId = uuidContainer.value();

        // Unlock the entity to send a new message
        if (this->lastUnacknowledgedMessage.has_value() && this->lastUnacknowledgedMessage.value().getId() == sentMessageId)
        {
            this->lastUnacknowledgedMessage = nullopt;
        }

        if (firstLine == "ACK-SYN")
            return this->receiveAckSynMessage(message, sentMessageId, uuidGenerator);
        else if (firstLine == "ACK-ACK-SYN")
            return this->receiveAckAckSynMessage(message, sentMessageId, uuidGenerator);
    }
    else
    {
        if (firstLine == "ACK-SYN")
            return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK-ACK-SYN\n" + to_string(message.getId()), Code::NACK);
        else if (firstLine == "ACK-ACK-SYN")
            return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK-ACK-ACK-SYN\n" + to_string(message.getId()), Code::NACK);
    }

    return nullopt;
}

optional<Message> Entity::receiveAckSynMessage(const Message &message, uuids::uuid sentMessageId, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    uuids::uuid synMessageId = sentMessageId;

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

optional<Message> Entity::receiveAckAckSynMessage(const Message &message, uuids::uuid sentMessageId, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    uuids::uuid ackSynMessageId = sentMessageId;

    auto connection = this->connections.find(message.getSourceEntityId());

    if (this->isConnectedTo(message.getSourceEntityId()) && connection->second->ackSynMessageId == ackSynMessageId)
    {
        // Update connection
        connection->second->ackAckSynMessageId = message.getId();
        return nullopt;
    }

    return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK-ACK-ACK-SYN\n" + to_string(message.getId()), Code::NACK);
}

optional<Message> Entity::receiveNackMessage(const Message &message, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    return nullopt;
}

optional<Message> Entity::receiveDataMessage(const Message &message, shared_ptr<uuids::uuid_random_generator> uuidGenerator)
{
    if (this->isConnectedTo(message.getSourceEntityId()))
    {
        this->storage += message.getContent() + "\n";

        return Message(uuidGenerator, this->id, message.getSourceEntityId(), "ACK\n" + to_string(message.getId()), Code::ACK);
    }

    return Message(uuidGenerator, this->id, message.getSourceEntityId(), "NACK\n" + to_string(message.getId()), Code::NACK);
}
