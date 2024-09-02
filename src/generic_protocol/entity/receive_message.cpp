#include <iostream>

#include "entity.hpp"
#include "package/package.hpp"

using namespace std;

Entity::Entity::Response Entity::receivePackage(
    Package package, shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();

    this->printPackageInformation(package, cout, false);

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          "NACK\n" + to_string(message.getId()),
                          Message::Code::NACK);

    if (package.isCorrupted()) return Response(error_message, false, nullopt);

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

    return Response(error_message, false, message.getId());
}

Entity::Response Entity::receiveSynMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          "NACK-SYN\n" + to_string(message.getId()),
                          Message::Code::NACK);
    Response error_response(error_message, false, message.getId());

    if (!this->isConnectedAtStep(
            // If there is no connection, create a new one
            {message.getSourceEntityId(), ConnectionStep::SYN})) {
        Message ack_syn_message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "ACK-SYN\n" + to_string(message.getId()), Message::Code::ACK);

        // Still need to receive the ACK-ACK-SYN message
        this->connect({message.getSourceEntityId(), message.getId(),
                       ConnectionStep::SYN});

        return Entity::Response(ack_syn_message, true, message.getId());

    } else {
        // If received a SYN message, but the connection is already established
        if (this->isConnectedAtStep(
                {message.getSourceEntityId(), ConnectionStep::SYN}))
            return Entity::Response{nullopt, false, message.getId()};
    }

    return error_response;
}

// TODO: Implement 3-way handshake
Entity::Response Entity::receiveFinMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          "NACK\n" + to_string(message.getId()),
                          Message::Code::NACK);
    Response error_response(error_message, false, message.getId());

    this->removeConnection({
        message.getSourceEntityId(),
    });
    return error_response;
}

Entity::Response Entity::receiveAckMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto ack_type_container = message.getAckType();
    auto uuid_container = message.getIdFromMessageBeingAcknowledged();

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          "NACK\n" + to_string(message.getId()),
                          Message::Code::NACK);
    Response error_response(error_message, false, message.getId());

    if (!ack_type_container.has_value()) return error_response;

    Message::AckType ack_type = ack_type_container.value();

    if (uuid_container.has_value()) {
        uuids::uuid sent_message_id = uuid_container.value();

        if (ack_type == Message::AckType::ACK_SYN)
            return this->receiveAckSynMessage(message, sent_message_id,
                                              uuid_generator);

        else if (ack_type == Message::AckType::ACK_ACK_SYN)
            return this->receiveAckAckSynMessage(message, sent_message_id,
                                                 uuid_generator);

        else
            return Response(nullopt, false, message.getId());

    } else {
        if (ack_type == Message::AckType::ACK_SYN)
            error_response.message =
                Message(uuid_generator, this->id, message.getSourceEntityId(),
                        "NACK-ACK-SYN\n" + to_string(message.getId()),
                        Message::Code::NACK);

        else if (ack_type == Message::AckType::ACK_ACK_SYN)
            error_response.message =
                Message(uuid_generator, this->id, message.getSourceEntityId(),
                        "NACK-ACK-ACK-SYN\n" + to_string(message.getId()),
                        Message::Code::NACK);
    }

    return error_response;
}

Entity::Response Entity::receiveAckSynMessage(
    const Message &message, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    Message ack_ack_syn_message(
        uuid_generator, this->id, message.getSourceEntityId(),
        "ACK-ACK-SYN\n" + to_string(message.getId()), Message::Code::ACK);

    this->connect({message.getSourceEntityId(), message.getId(),
                   ConnectionStep::ACK_SYN});

    return Response(ack_ack_syn_message, true, message.getId());
}

Entity::Response Entity::receiveAckAckSynMessage(
    const Message &message, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    if (this->isConnectedAtStep(
            {message.getSourceEntityId(), ConnectionStep::ACK_SYN})) {
        // Update connection
        this->connect({message.getSourceEntityId(), message.getId(),
                       ConnectionStep::ACK_ACK_SYN});

        Message ack_message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "ACK\n" + to_string(message.getId()), Message::Code::ACK);
        return Response(ack_message, false, message.getId());
    }

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          "NACK-ACK-ACK-SYN\n" + to_string(message.getId()),
                          Message::Code::NACK);
    Response error_response{error_message, false, message.getId()};
    return error_response;
}

Entity::Response Entity::receiveNackMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return Response({nullopt, false, message.getId()});
}

Entity::Response Entity::receiveDataMessage(
    const Message &message,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          "NACK\n" + to_string(message.getId()),
                          Message::Code::NACK);
    Response error_response(error_message, false, message.getId());

    if (this->canStoreData({message.getSourceEntityId(), message.getId()})) {
        this->storage += message.getDataContent() + "\n";

        Message ack_message(
            uuid_generator, this->id, message.getSourceEntityId(),
            "ACK\n" + to_string(message.getId()), Message::Code::ACK);

        return Response(ack_message, false, message.getId());
    }

    return error_response;
}
