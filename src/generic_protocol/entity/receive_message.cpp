#include <iostream>
#include <optional>

#include "entity.hpp"
#include "message.hpp"
#include "package.hpp"

using namespace std;

optional<Package> Entity::receivePackage(
    Package package, shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();

    this->printPackageInformation(package, cout, false);

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          Message::Code::NACK, nullopt, nullopt);
    Package error_package(error_message, false);

    if (package.isCorrupted()) return error_package;

    switch (message.getCode()) {
        case Message::Code::SYN:
            return this->receiveSynPackage(package, uuid_generator);
        case Message::Code::FIN:
            return this->receiveFinPackage(package, uuid_generator);
        case Message::Code::ACK:
            return this->receiveAckPackage(package, uuid_generator);
        case Message::Code::NACK:
            return this->receiveNackPackage(package, uuid_generator);
        case Message::Code::DATA:
            return this->receiveDataPackage(package, uuid_generator);
    }

    // Received package successfully, but it cannot be processed
    error_package.setIdFromMessageBeingAcknowledged(message.getId());
    return error_package;
}

optional<Package> Entity::receiveSynPackage(
    const Package &package,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();

    if (!this->isConnectedAtStep(
            // If there is no connection, create a new one
            {message.getSourceEntityId(), ConnectionStep::SYN})) {
        Message ack_syn_message(uuid_generator, this->id,
                                message.getSourceEntityId(), Message::Code::ACK,
                                Message::CodeVariant::ACK_SYN, message.getId());

        // Still need to receive the ACK-ACK-SYN message
        this->connect({message.getSourceEntityId(), message.getId(),
                       ConnectionStep::SYN});

        return Package(ack_syn_message, true);
    }
    // else {
    //     // If received a SYN message, but the connection is already
    //     established if (this->isConnectedAtStep(
    //             {message.getSourceEntityId(), ConnectionStep::SYN}))
    //         return error_response;
    // }

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          Message::Code::NACK, Message::CodeVariant::NACK_SYN,
                          message.getId());
    Package error_package(error_message, false);
    return error_package;
}

// TODO: Implement 3-way handshake
optional<Package> Entity::receiveFinPackage(
    const Package &package,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();
    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          Message::Code::NACK, Message::CodeVariant::NACK_FIN,
                          message.getId());
    Package error_package(error_message, false);

    bool is_connected = this->isConnectedAtStep(
        {message.getSourceEntityId(), ConnectionStep::SYN});

    if (!is_connected) return error_package;

    this->removeConnection({
        message.getSourceEntityId(),
    });

    return Package(Message(uuid_generator, this->id,
                           message.getSourceEntityId(), Message::Code::ACK,
                           Message::CodeVariant::ACK_FIN, message.getId()),
                   true);
}

optional<Package> Entity::receiveAckPackage(
    const Package &package,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          Message::Code::NACK, nullopt, message.getId());

    auto variant = message.getCodeVariant();

    auto previously_sent_message_id =
        message.getIdFromMessageBeingAcknowledged();

    if (variant.has_value()) {
        if (previously_sent_message_id.has_value()) {
            if (variant.value() == Message::CodeVariant::ACK_SYN)
                return this->receiveAckSynPackage(
                    package, previously_sent_message_id.value(),
                    uuid_generator);

            else if (variant.value() == Message::CodeVariant::ACK_ACK_SYN)
                return this->receiveAckAckSynPackage(
                    package, previously_sent_message_id.value(),
                    uuid_generator);

            else if (variant.value() == Message::CodeVariant::ACK_ACK_ACK_SYN)
                return nullopt;

        } else {  // Wrongfully received an ACK message
            if (variant.value() == Message::CodeVariant::ACK_SYN)
                error_message.setCodeVariant(
                    Message::CodeVariant::NACK_ACK_SYN);

            else if (variant.value() == Message::CodeVariant::ACK_ACK_SYN)
                error_message.setCodeVariant(
                    Message::CodeVariant::NACK_ACK_ACK_SYN);
        }
    }

    return Package(error_message, false);
}

optional<Package> Entity::receiveAckSynPackage(
    const Package &package, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();
    Message ack_ack_syn_message(
        uuid_generator, this->id, message.getSourceEntityId(),
        Message::Code::ACK, Message::CodeVariant::ACK_ACK_SYN, message.getId());

    this->connect({message.getSourceEntityId(), message.getId(),
                   ConnectionStep::ACK_SYN});

    return Package(ack_ack_syn_message, true);
}

optional<Package> Entity::receiveAckAckSynPackage(
    const Package &package, uuids::uuid sent_message_id,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();

    if (this->isConnectedAtStep(
            {message.getSourceEntityId(), ConnectionStep::ACK_SYN})) {
        // Update connection
        this->connect({message.getSourceEntityId(), message.getId(),
                       ConnectionStep::ACK_ACK_SYN});

        Message ack_message(uuid_generator, this->id,
                            message.getSourceEntityId(), Message::Code::ACK,
                            Message::CodeVariant::ACK_ACK_ACK_SYN,
                            message.getId());
        return Package(ack_message, false);
    }

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          Message::Code::NACK,
                          Message::CodeVariant::NACK_ACK_ACK_SYN,
                          message.getId());
    return Package(error_message, false);
}

// TODO: Split confirmation from received and from processed
optional<Package> Entity::receiveNackPackage(
    const Package &package,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return nullopt;
}

optional<Package> Entity::receiveDataPackage(
    const Package &package,
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    auto message = package.getMessage();

    Message error_message(uuid_generator, this->id, message.getSourceEntityId(),
                          Message::Code::NACK, nullopt, nullopt);

    if (this->canStoreData({message.getSourceEntityId(), message.getId()})) {
        this->dequeuePackage({message.getSourceEntityId()});

        this->storage += message.getContent() + "\n";

        Message ack_message(uuid_generator, this->id,
                            message.getSourceEntityId(), Message::Code::ACK,
                            nullopt, message.getId());

        return Package(ack_message, false);
    }

    return Package(error_message, false);
}
