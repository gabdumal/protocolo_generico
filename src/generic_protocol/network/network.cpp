#include "network.hpp"

#include <generic_protocol_constants.hpp>
#include <iostream>
#include <pretty_console.hpp>

#include "util.hpp"

using namespace std;

/* Construction */

Network::Network(string name,
                 shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    this->uuid_generator = uuid_generator;
    this->name = name;
    this->processing_messages_count = 0;
    this->can_stop_processing_thread = false;
    this->processing_messages_thread = thread([this]() {
        while (true) {
            unique_lock<mutex> lock(this->messages_to_process_mutex);
            if (this->can_stop_processing_thread &&
                messages_to_process.empty()) {
                break;
            }
            if (!this->messages_to_process.empty()) {
                Message message = this->messages_to_process.front();
                this->messages_to_process.pop();
                lock.unlock();
                this->processMessage(message);
                lock.lock();
            } else {
                this->message_processed_cv.wait(lock);
            }
        }
    });
}

Network::~Network() {
    {
        lock_guard<mutex> lock(this->messages_to_process_mutex);
        this->can_stop_processing_thread = true;
        this->message_processed_cv.notify_all();
    }
    this->joinThread();
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
    this->message_processed_cv.notify_all();
}

void Network::disconnectEntity(uuids ::uuid entity_id) {
    lock_guard<mutex> lock(this->entities_mutex);
    this->entities.erase(entity_id);
    this->message_processed_cv.notify_all();
}

/* Main */

bool Network::receiveMessage(Message message) {
    this->registerMessageSending(message);
    return this->preprocessMessage(message);
}

bool Network::preprocessMessage(Message message) {
    if (this->hasPackageBeenLost(message.getId())) return false;
    return this->insertMessageIntoProcessingQueue(message);
}

void Network::processMessage(Message message) {
    this->simulateNetworkLatency();
    this->simulatePacketCorruption(message);

    this->sendMessage(message);
    this->finishMessageProcessing();
}

/* Operational */

void Network::sendMessage(Message message) {
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
                    bool can_send_message = source_entity->sendMessage(message);

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
                                "Message " + to_string(message.getId()) +
                                    " has been sent to the target entity " +
                                    target_entity->getName() + " [" +
                                    to_string(target_entity->getId()) + "]!",
                                cout, PrettyConsole::Color::GREEN);
                        }

                        optional<Message> returned_message =
                            target_entity->receiveMessage(message,
                                                          this->uuid_generator);

                        if (returned_message) {
                            this->receiveMessage(returned_message.value());
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
    lock_guard<mutex> lock(this->messages_to_process_mutex);
    this->processing_messages_count--;
    if (processing_messages_count == 0) {
        this->message_processed_cv.notify_all();  // Notify the destructor
    }
}

/* Auxiliary */

bool Network::insertMessageIntoProcessingQueue(Message message) {
    try {
        lock_guard<mutex> lock(this->messages_to_process_mutex);
        this->messages_to_process.push(message);
        this->processing_messages_count++;
        this->message_processed_cv.notify_one();  // Notify the network thread
        return true;
    } catch (const exception &e) {
        this->printInformation("Error while receiving message in the network " +
                                   this->getName() + ": " + e.what(),
                               cerr, PrettyConsole::Color::RED);
        return false;
    }
}

void Network::joinThread() {
    if (this->processing_messages_thread.joinable()) {
        this->processing_messages_thread.join();
    }
}

void Network::registerMessageSending(Message message) {
    lock_guard<mutex> lock(this->messages_to_process_mutex);
    this->unconfirmed_messages.insert(
        {message.getId(), MessageSending(message)});
}

bool Network::hasPackageBeenLost(uuids::uuid message_id) {
    if (rand() % 100 <
        GenericProtocolConstants::packet_loss_probability * 100) {
        if (GenericProtocolConstants::debug_information) {
            this->printInformation("Message " + to_string(message_id) +
                                       " has been lost in the network " +
                                       this->getName() + "!",
                                   cout, PrettyConsole::Color::YELLOW);
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
                               cout, PrettyConsole::Color::YELLOW);
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

/* Static Methods */

unique_ptr<Network> Network::createNetwork(
    string name, shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return unique_ptr<Network>(new Network(name, uuid_generator));
}
