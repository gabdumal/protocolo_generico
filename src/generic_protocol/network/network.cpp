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
    this->can_stop_thread = false;
    this->network_thread = thread([this]() {
        while (true) {
            unique_lock<mutex> lock(this->messages_mutex);
            if (this->can_stop_thread && messages.empty()) {
                break;
            }
            if (!this->messages.empty()) {
                Message message = this->messages.front();
                this->messages.pop();
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
        lock_guard<mutex> lock(this->messages_mutex);
        this->can_stop_thread = true;
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

/* Methods */

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

bool Network::receiveMessage(Message message) {
    // Simulate packet loss
    if (rand() % 100 <
        GenericProtocolConstants::packet_loss_probability * 100) {
        if (GenericProtocolConstants::debug_information) {
            this->printInformation("Message " + to_string(message.getId()) +
                                       " has been lost in the network " +
                                       this->getName() + "!",
                                   cout, PrettyConsole::Color::YELLOW);
        }
        return false;
    }

    try {
        lock_guard<mutex> lock(this->messages_mutex);
        this->messages.push(message);
        this->processing_messages_count++;
        this->message_processed_cv.notify_one();  // Notify the network thread
    } catch (const exception &e) {
        this->printInformation("Error while receiving message in the network " +
                                   this->getName() + ": " + e.what(),
                               cerr, PrettyConsole::Color::RED);
        return false;
    }
    return true;
}

void Network::processMessage(Message message) {
    // Simulate network latency
    chrono::milliseconds time_span(rand() %
                                   GenericProtocolConstants::network_latency);
    this_thread::sleep_for(time_span);

    // Simulate message corruption
    if (rand() % 100 <
        GenericProtocolConstants::packet_corruption_probability * 100) {
        this->printInformation("Message [" + to_string(message.getId()) +
                                   "] has been corrupted in the network " +
                                   this->getName() + "!",
                               cout, PrettyConsole::Color::YELLOW);
        message.setCorrupted(true);
    }

    this->sendMessage(message);

    // Brackets are used to limit the scope of the lock
    {
        lock_guard<mutex> lock(this->messages_mutex);
        this->processing_messages_count--;
        if (processing_messages_count == 0) {
            this->message_processed_cv.notify_all();  // Notify the destructor
        }
    }
}

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

void Network::joinThread() {
    if (this->network_thread.joinable()) {
        this->network_thread.join();
    }
}

/* Static Methods */

unique_ptr<Network> Network::createNetwork(
    string name, shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    return unique_ptr<Network>(new Network(name, uuid_generator));
}
