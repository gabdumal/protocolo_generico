#include "network.hpp"

#include <generic_protocol_constants.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <pretty_console.hpp>

#include "message.hpp"
#include "util.hpp"

using namespace std;

/* Construction */

Network::Network(string name,
                 shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    this->uuid_generator = uuid_generator;
    this->name = name;

    this->unconfirmed_messages =
        make_shared<map<uuids::uuid, MessageSending>>();
    this->sending_messages_count = 0;
    this->can_stop_sending_thread = false;
    this->message_sending_thread =
        thread([this]() { this->sendingThreadJob(); });

    this->packages_to_process = make_shared<queue<Package>>();
    this->processing_packages_count = 0;
    this->can_stop_processing_thread = false;
    this->processing_packages_thread =
        thread([this]() { this->processingThreadJob(); });
}

Network::~Network() {
    this->joinThreads();
    if (GenericProtocolConstants::debug_information) {
        this->printInformation(
            "Network " + this->getName() + " has been destroyed!", cout,
            PrettyConsole::Color::YELLOW);
    }
}

/* Getters */

string Network::getName() const { return name; }

/* Entities */

void Network::connectEntity(shared_ptr<Entity> entity) {
    lock_guard<mutex> lock(this->entities_mutex);
    this->entities[entity->getId()] = entity;
    this->package_processed_cv.notify_all();
}

void Network::disconnectEntity(uuids ::uuid entity_id) {
    lock_guard<mutex> lock(this->entities_mutex);
    this->entities.erase(entity_id);
    this->package_processed_cv.notify_all();
}

/* Main */

bool Network::receiveMessage(Message message) {
    Package package(message, true);
    return this->receivePackage(package);
}

bool Network::receivePackage(Package package) {
    if (GenericProtocolConstants::debug_information) {
        this->printInformation(
            "Package [" + to_string(package.message.getId()) +
                "] has been received in the network " + this->getName() + "!",
            cout, PrettyConsole::Color::GREEN);
    }

    this->registerPackage(package);
    return this->preprocessPackage(package);
}

bool Network::preprocessPackage(Package package, int attempt) {
    auto message = package.message;

    if (GenericProtocolConstants::debug_information) {
        this->printInformation(
            "Attempt [" + to_string(attempt) + "] to send message [" +
                to_string(message.getId()) + "] to the target [" +
                to_string(message.getTargetEntityId()) + "]",
            cout, PrettyConsole::Color::YELLOW);
    }

    if (this->hasPackageBeenLost(message.getId())) return false;
    return this->insertPackageIntoProcessingQueue(package);
}

void Network::processMessage(Package package) {
    this->simulateNetworkLatency();
    this->simulatePacketCorruption(package.message);

    this->sendPackage(package);
    this->finishMessageProcessing();
}

/* Operational */

void Network::sendPackage(Package &package) {
    auto message = package.message;
    bool should_be_confirmed = package.should_be_confirmed;

    lock_guard<mutex> lock(this->entities_mutex);
    auto target_entity_pair = this->entities.find(message.getTargetEntityId());

    if (target_entity_pair != entities.end()) {
        shared_ptr<Entity> target_entity = target_entity_pair->second;

        if (target_entity) {
            auto source_entity_pair =
                this->entities.find(message.getSourceEntityId());

            if (source_entity_pair != entities.end()) {
                shared_ptr<Entity> source_entity = source_entity_pair->second;

                if (source_entity) {
                    bool can_send_message = source_entity->sendMessage(
                        message, should_be_confirmed);

                    if (!can_send_message) {
                        if (GenericProtocolConstants::debug_information) {
                            this->printInformation(
                                "Source entity " + source_entity->getName() +
                                    " [" + to_string(source_entity->getId()) +
                                    "] cannot send the message " + "[" +
                                    to_string(message.getId()) + "]!",
                                cout, PrettyConsole::Color::RED);
                        }
                    } else {
                        if (GenericProtocolConstants::debug_information) {
                            this->printInformation(
                                "Message [" + to_string(message.getId()) +
                                    "] has been sent to the target entity " +
                                    target_entity->getName() + " [" +
                                    to_string(target_entity->getId()) + "]!",
                                cout, PrettyConsole::Color::GREEN);
                        }

                        Entity::Response response =
                            target_entity->receiveMessage(message,
                                                          this->uuid_generator);

                        this->tryToConfirmSomeMessage(
                            response.id_from_message_possibly_acknowledged);

                        optional<Message> returned_message = response.message;
                        if (returned_message) {
                            if (GenericProtocolConstants::debug_information) {
                                this->printInformation(
                                    "Response message [" +
                                        to_string(message.getId()) +
                                        "] has been received in the network " +
                                        this->getName() + "!",
                                    cout, PrettyConsole::Color::GREEN);
                            }
                            this->receivePackage(
                                Package(returned_message.value(),
                                        response.should_be_confirmed));
                        }
                    }
                } else {
                    this->printInformation(
                        "Source entity [" +
                            to_string(message.getSourceEntityId()) +
                            "] has been disconnected from the network " +
                            this->getName() + "!",
                        cerr, PrettyConsole::Color::RED);
                    this->disconnectEntity(message.getSourceEntityId());
                }
            } else {
                this->printInformation(
                    "Source entity [" + to_string(message.getSourceEntityId()) +
                        "] is not connected to the network " + this->getName() +
                        "!",
                    cerr, PrettyConsole::Color::RED);
            }
        } else {
            this->printInformation(
                "Target entity [" + to_string(message.getTargetEntityId()) +
                    "] has been disconnected from the network " +
                    this->getName() + "!",
                cerr, PrettyConsole::Color::RED);
            this->disconnectEntity(message.getTargetEntityId());
        }
    } else {
        this->printInformation(
            "Target entity [" + to_string(message.getTargetEntityId()) +
                "] is not connected to the network " + this->getName() + "!",
            cerr, PrettyConsole::Color::RED);
    }
}

void Network::finishMessageProcessing() {
    lock_guard<mutex> lock(this->packages_to_process_mutex);
    this->processing_packages_count--;
    if (processing_packages_count == 0) {
        this->package_processed_cv.notify_all();  // Notify the destructor
    }
}

/* Auxiliary */

bool Network::insertPackageIntoProcessingQueue(Package package) {
    try {
        lock_guard<mutex> lock(this->packages_to_process_mutex);
        this->packages_to_process->push(package);
        this->processing_packages_count++;
        this->package_processed_cv.notify_one();  // Notify the network thread
        return true;
    } catch (const exception &e) {
        this->printInformation("Error while receiving message in the network " +
                                   this->getName() + ": " + e.what(),
                               cerr, PrettyConsole::Color::RED);
        return false;
    }
}

void Network::joinSendingThread() {
    if (this->message_sending_thread.joinable()) {
        this->message_sending_thread.join();
    }
}

void Network::joinProcessingThread() {
    if (this->processing_packages_thread.joinable()) {
        this->processing_packages_thread.join();
    }
}

void Network::joinThreads() {
    {
        lock_guard<mutex> lock(this->unconfirmed_messages_mutex);
        this->can_stop_sending_thread = true;
        this->message_sent_cv.notify_all();
    }
    this->joinSendingThread();
    {
        lock_guard<mutex> lock(this->packages_to_process_mutex);
        this->can_stop_processing_thread = true;
        this->package_processed_cv.notify_all();
    }
    this->joinProcessingThread();
}

void Network::registerPackage(Package package) {
    auto message = package.message;
    bool should_be_confirmed = package.should_be_confirmed;

    auto source_entity_pair = this->entities.find(message.getSourceEntityId());
    if (source_entity_pair == this->entities.end()) {
        this->printInformation(
            "Source entity [" + to_string(message.getSourceEntityId()) +
                "] is not connected to the network " + this->getName() + "!",
            cerr, PrettyConsole::Color::RED);
    }

    auto source_entity = source_entity_pair->second;
    source_entity->printMessageInformation(message, cout, true);

    if (!should_be_confirmed) return;

    lock_guard<mutex> lock(this->unconfirmed_messages_mutex);
    this->unconfirmed_messages->insert(
        {message.getId(), MessageSending(message)});
}

bool Network::hasPackageBeenLost(uuids::uuid message_id) {
    if (rand() % 100 <
        GenericProtocolConstants::packet_loss_probability * 100) {
        if (GenericProtocolConstants::debug_information) {
            this->printInformation("Message [" + to_string(message_id) +
                                       "] has been lost in the network " +
                                       this->getName() + "!",
                                   cout, PrettyConsole::Color::RED);
        }
        return true;
    }
    return false;
}

void Network::simulateNetworkLatency() {
    chrono::milliseconds time_span(rand() %
                                   GenericProtocolConstants::network_latency);
    this_thread::sleep_for(time_span);
}

void Network::simulatePacketCorruption(Message &message) {
    if (rand() % 100 <
        GenericProtocolConstants::packet_corruption_probability * 100) {
        this->printInformation("Message [" + to_string(message.getId()) +
                                   "] has been corrupted in the network " +
                                   this->getName() + "!",
                               cout, PrettyConsole::Color::RED);
        message.setCorrupted(true);
    }
}

void Network::printInformation(string information, ostream &output_stream,
                               PrettyConsole::Color color) const {
    string header = "Network " + this->getName();
    PrettyConsole::Decoration header_decoration{PrettyConsole::Color::BLACK,
                                                PrettyConsole::Color::YELLOW,
                                                PrettyConsole::Format::BOLD};
    PrettyConsole::Decoration information_decoration{
        color, PrettyConsole::Color::DEFAULT, PrettyConsole::Format::NONE};
    Util::printInformation(header, information, output_stream,
                           header_decoration, information_decoration);
}

void Network::tryToConfirmSomeMessage(
    optional<uuids::uuid> id_from_message_possibly_acknowledged) {
    if (id_from_message_possibly_acknowledged)
        this->removeMessageFromUnconfirmedMessages(
            id_from_message_possibly_acknowledged.value());
}

void Network::removeMessageFromUnconfirmedMessages(uuids::uuid message_id) {
    lock_guard<mutex> lock(this->unconfirmed_messages_mutex);
    auto it = this->unconfirmed_messages->find(message_id);

    if (it != this->unconfirmed_messages->end()) {
        if (GenericProtocolConstants::debug_information) {
            this->printInformation(
                "Message [" + to_string(message_id) + "] has been confirmed!",
                cout, PrettyConsole::Color::GREEN);
        }
        this->unconfirmed_messages->erase(it);
    }
}

/* Thread jobs */

void Network::sendingThreadJob() {
    while (true) {
        unique_lock<mutex> lock(this->unconfirmed_messages_mutex);

        // Finish job if there are no messages to send
        if (this->can_stop_sending_thread && unconfirmed_messages->empty()) {
            break;
        }

        for (auto it = this->unconfirmed_messages->begin();
             it != this->unconfirmed_messages->end();) {
            MessageSending &message_sending = it->second;

            // Check if the timeout has expired
            if (chrono::system_clock::now() -
                    message_sending.last_attempt_time >
                GenericProtocolConstants::resend_timeout) {
                // Check if the message has remaining attempts
                if (message_sending.remaining_attempts > 0) {
                    message_sending.last_attempt_time =
                        chrono::system_clock::now();
                    message_sending.remaining_attempts--;
                    lock.unlock();
                    this->preprocessPackage(
                        Package(message_sending.message, true),
                        GenericProtocolConstants::max_attempts_to_send_message -
                            message_sending.remaining_attempts);
                    lock.lock();
                } else {
                    // Finished attempts to send the message
                    if (GenericProtocolConstants::debug_information) {
                        this->printInformation(
                            "Message [" +
                                to_string(message_sending.message.getId()) +
                                "] has been removed from the network " +
                                this->getName() + "!",
                            cout, PrettyConsole::Color::RED);
                    }
                    it = this->unconfirmed_messages->erase(it);
                }
            } else {
                it++;
            }
        }

        // Wait for a message to send
        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(
            GenericProtocolConstants::interval_to_check_unconfirmed_messages));
    }
}

void Network::processingThreadJob() {
    while (true) {
        unique_lock<mutex> lock(this->packages_to_process_mutex);

        // Finish job if there are no messages to process
        if (this->can_stop_processing_thread && packages_to_process->empty()) {
            break;
        }

        if (!this->packages_to_process->empty()) {
            auto package = this->packages_to_process->front();
            this->packages_to_process->pop();
            lock.unlock();
            this->processMessage(package);
            lock.lock();
        } else {
            // Wait for a message to process
            this->package_processed_cv.wait(lock);
        }
    }
}

/* Static Methods */

unique_ptr<Network> Network::createNetwork(
    string name, shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return unique_ptr<Network>(new Network(name, uuid_generator));
}
