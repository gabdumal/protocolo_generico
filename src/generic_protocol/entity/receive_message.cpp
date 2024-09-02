#include <iostream>

#include "entity.hpp"

using namespace std;

Entity::Entity::Response Entity::receiveMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    this->printMessageInformation(message, cout, false);

    if (message.isCorrupted())
        return Response{Message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "NACK\n" + to_string(message.getId()), Message::Code::NACK)};

    switch (message.getCode()) {
        case Message::Code::SYN:
            return this->receiveSynMessage(message, uuid_generator);
        case Message::Code::FIN:
            return this->receiveFinMessage(message, uuid_generator);
        case Message::Code::ACK:
            return this->receiveAckMessage(message, uuid_generator);
        case Message::Code::NACK:
            return this->receiveNackMessage(message, uuid_generator);
        case Message::Code::DATA:
            return this->receiveDataMessage(message, uuid_generator);
    }

    return Response();
}

Entity::Response Entity::receiveSynMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    if (this->isConnectedAtStep(
            {message.getSourceEntityId(), ConnectionStep::NONE})) {
        Message ack_syn_message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "ACK-SYN\n" + to_string(message.getId()), Message::Code::ACK);

        // Still need to receive the ACK-ACK-SYN message
        this->connect({message.getSourceEntityId(), message.getId(),
                       ConnectionStep::SYN});

        return Entity::Response(ack_syn_message);
    } else {
        if (this->isConnectedAtStep(
                {message.getSourceEntityId(), ConnectionStep::SYN}))
            return Entity::Response();

        return Entity::Response{Message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "NACK-SYN\n" + to_string(message.getId()), Message::Code::NACK)};
    }
}

// TODO: Implement 3-way handshake
Entity::Response Entity::receiveFinMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    this->removeConnection({
        message.getSourceEntityId(),
    });
    return Response();
}

Entity::Response Entity::receiveAckMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto ack_type_container = message.getAckType();
    auto uuid_container = message.getIdFromMessageBeingAcknowledged();

    if (!ack_type_container.has_value()) return Response();

    Message::AckType ack_type = ack_type_container.value();

    if (uuid_container.has_value()) {
        uuids::uuid sent_message_id = uuid_container.value();

        // Unlock the entity to send a new message
        if (this->last_unacknowledged_message.has_value() &&
            this->last_unacknowledged_message.value().getId() ==
                sent_message_id)
            this->last_unacknowledged_message = nullopt;

        if (ack_type == Message::AckType::ACK_SYN)
            return this->receiveAckSynMessage(message, sent_message_id,
                                              uuid_generator);

        else if (ack_type == Message::AckType::ACK_ACK_SYN)
            return this->receiveAckAckSynMessage(message, sent_message_id,
                                                 uuid_generator);

    } else {
        if (ack_type == Message::AckType::ACK_SYN)
            return Response{
                Message(uuid_generator, this->id, message.getSourceEntityId(),
                        "NACK-ACK-SYN\n" + to_string(message.getId()),
                        Message::Code::NACK)};

        else if (ack_type == Message::AckType::ACK_ACK_SYN)
            return Response{
                Message(uuid_generator, this->id, message.getSourceEntityId(),
                        "NACK-ACK-ACK-SYN\n" + to_string(message.getId()),
                        Message::Code::NACK)};
    }

    return Response();
}

Entity::Response Entity::receiveAckSynMessage(
    const Message &message, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    uuids::uuid syn_message_id = sent_message_id;

    Message ack_ack_syn_message(
        uuid_generator, this->id, message.getSourceEntityId(),
        "ACK-ACK-SYN\n" + to_string(message.getId()), Message::Code::ACK);

    this->connect({message.getSourceEntityId(), message.getId(),
                   ConnectionStep::ACK_SYN});

    return Response(ack_ack_syn_message);
}

Entity::Response Entity::receiveAckAckSynMessage(
    const Message &message, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    uuids::uuid ack_syn_message_id = sent_message_id;

    if (this->isConnectedAtStep(
            {message.getSourceEntityId(), ConnectionStep::ACK_SYN})) {
        // Update connection
        this->connect({message.getSourceEntityId(), message.getId(),
                       ConnectionStep::ACK_ACK_SYN});

        Message ack_ack_ack_syn_message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "ACK-ACK-ACK-SYN\n" + to_string(message.getId()),
            Message::Code::ACK);

        return Response(ack_ack_ack_syn_message);
    }

    return Response{Message(uuid_generator, this->id,
                            message.getSourceEntityId(),
                            "NACK-ACK-ACK-SYN\n" + to_string(message.getId()),
                            Message::Code::NACK)};
}

Entity::Response Entity::receiveAckAckAckSynMessage(
    const Message &message, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return Response();
}

Entity::Response Entity::receiveNackMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return Response();
}

Entity::Response Entity::receiveDataMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    if (this->isConnectedAtStep(
            {message.getSourceEntityId(), ConnectionStep::ACK_ACK_SYN})) {
        this->storage += message.getContent() + "\n";

        return Response{
            Message(uuid_generator, this->id, message.getSourceEntityId(),
                    "ACK\n" + to_string(message.getId()), Message::Code::ACK)};
    }

    return Response{
        Message(uuid_generator, this->id, message.getSourceEntityId(),
                "NACK\n" + to_string(message.getId()), Message::Code::NACK)};
}
