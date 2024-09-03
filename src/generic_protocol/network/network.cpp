#include "network.hpp"

#include <functional>
#include <generic_protocol_constants.hpp>
#include <iostream>
#include <memory>
#include <pretty_console.hpp>

#include "message.hpp"
#include "util.hpp"

using namespace std;

/* Construction */

Network::Network(string name,
                 shared_ptr<uuids::uuid_random_generator> uuid_generator,
                 function<shared_ptr<Entity>(uuids::uuid)> get_entity_by_id) {
    this->uuid_generator = uuid_generator;
    this->name = name;
    this->get_entity_by_id = get_entity_by_id;

    this->unconfirmed_packages =
        make_shared<map<uuids::uuid, PackageSending>>();
    this->sending_packages_count = 0;
    this->can_stop_sending_thread = false;
    this->package_sending_thread =
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

shared_ptr<Entity> Network::getEntityById(uuids::uuid entity_id) {
    return this->get_entity_by_id(entity_id);
}

/* Main */

bool Network::receivePackage(Package package) {
    return this->internalReceivePackage(package);
}

bool Network::internalReceivePackage(Package package) {
    if (GenericProtocolConstants::debug_information) {
        this->printInformation(
            "Package [" + to_string(package.getMessage().getId()) +
                "] has been received in the network " + this->getName() + "!",
            cout, PrettyConsole::Color::GREEN);
    }

    this->registerPackage(package);
    return this->preprocessPackage(package);
}

bool Network::preprocessPackage(Package package, int attempt) {
    auto message = package.getMessage();

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

void Network::processPackage(Package package) {
    this->simulateNetworkLatency();
    this->simulatePacketCorruption(package);

    this->sendPackage(package);
    this->finishPackageProcessing();
}

/* Operational */

void Network::sendPackage(Package &package) {
    auto message = package.getMessage();
    bool should_be_confirmed = package.shouldBeConfirmed();

    auto target_entity = this->getEntityById(message.getTargetEntityId());
    if (!target_entity) {
        this->printInformation(
            "Target entity [" + to_string(message.getTargetEntityId()) +
                "] is not connected to the network " + this->getName() + "!",
            cerr, PrettyConsole::Color::RED);
        return;
    }
    auto source_entity = this->getEntityById(message.getSourceEntityId());
    if (!source_entity) {
        this->printInformation(
            "Source entity [" + to_string(message.getSourceEntityId()) +
                "] is not connected to the network " + this->getName() + "!",
            cerr, PrettyConsole::Color::RED);
    }

    bool can_send_message =
        source_entity->sendMessage(message, should_be_confirmed);

    if (!can_send_message) {
        if (GenericProtocolConstants::debug_information) {
            this->printInformation("Source entity " + source_entity->getName() +
                                       " [" +
                                       to_string(source_entity->getId()) +
                                       "] cannot send the message " + "[" +
                                       to_string(message.getId()) + "]!",
                                   cout, PrettyConsole::Color::RED);
        }
    } else {
        if (GenericProtocolConstants::debug_information) {
            this->printInformation("Message [" + to_string(message.getId()) +
                                       "] has been sent to the target entity " +
                                       target_entity->getName() + " [" +
                                       to_string(target_entity->getId()) + "]!",
                                   cout, PrettyConsole::Color::GREEN);
        }

        optional<Package> returned_package_container =
            target_entity->receivePackage(package, this->uuid_generator);

        if (!returned_package_container.has_value()) return;

        auto returned_package = returned_package_container.value();

        this->tryToConfirmSomePackage(
            returned_package.getMessage().getIdFromMessageBeingAcknowledged());

        if (GenericProtocolConstants::debug_information) {
            this->printInformation("Response message [" +
                                       to_string(message.getId()) +
                                       "] has been received in the network " +
                                       this->getName() + "!",
                                   cout, PrettyConsole::Color::GREEN);
        }

        this->internalReceivePackage(returned_package);
    }
}

void Network::finishPackageProcessing() {
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
    if (this->package_sending_thread.joinable()) {
        this->package_sending_thread.join();
    }
}

void Network::joinProcessingThread() {
    if (this->processing_packages_thread.joinable()) {
        this->processing_packages_thread.join();
    }
}

void Network::joinThreads() {
    {
        lock_guard<mutex> lock_sending(this->unconfirmed_packages_mutex);
        this->can_stop_sending_thread = true;
        this->package_sent_cv.notify_all();
    }
    this->joinSendingThread();

    {
        lock_guard<mutex> lock_processing(this->packages_to_process_mutex);
        this->can_stop_processing_thread = true;
        this->package_processed_cv.notify_all();
    }
    this->joinProcessingThread();
}

void Network::registerPackage(Package package) {
    auto message = package.getMessage();
    bool should_be_confirmed = package.shouldBeConfirmed();

    auto source_entity = this->getEntityById(message.getSourceEntityId());

    if (source_entity == nullptr) {
        this->printInformation(
            "Source entity [" + to_string(message.getSourceEntityId()) +
                "] is not connected to the network " + this->getName() + "!",
            cerr, PrettyConsole::Color::RED);
        return;
    }

    source_entity->printPackageInformation(package, cout, true);

    if (!should_be_confirmed) return;

    lock_guard<mutex> lock(this->unconfirmed_packages_mutex);
    this->unconfirmed_packages->insert(
        {message.getId(), PackageSending(package)});
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

void Network::simulatePacketCorruption(Package &package) {
    if (rand() % 100 <
        GenericProtocolConstants::packet_corruption_probability * 100) {
        this->printInformation(
            "Message [" + to_string(package.getMessage().getId()) +
                "] has been corrupted in the network " + this->getName() + "!",
            cout, PrettyConsole::Color::RED);
        package.setCorrupted(true);
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

void Network::tryToConfirmSomePackage(
    optional<uuids::uuid> id_from_package_possibly_acknowledged) {
    if (id_from_package_possibly_acknowledged)
        this->removePackageFromUnconfirmedPackages(
            id_from_package_possibly_acknowledged.value());
}

void Network::removePackageFromUnconfirmedPackages(uuids::uuid package_id) {
    lock_guard<mutex> lock(this->unconfirmed_packages_mutex);
    auto it = this->unconfirmed_packages->find(package_id);

    if (it != this->unconfirmed_packages->end()) {
        if (GenericProtocolConstants::debug_information) {
            this->printInformation(
                "Message [" + to_string(package_id) + "] has been confirmed!",
                cout, PrettyConsole::Color::GREEN);
        }
        this->unconfirmed_packages->erase(it);
    }
}

/* Thread jobs */

void Network::sendingThreadJob() {
    while (true) {
        unique_lock<mutex> lock(this->unconfirmed_packages_mutex);

        // Finish job if there are no packages to send
        if (this->can_stop_sending_thread && unconfirmed_packages->empty()) {
            break;
        }

        for (auto it = this->unconfirmed_packages->begin();
             it != this->unconfirmed_packages->end();) {
            PackageSending &package_sending = it->second;

            // Check if the timeout has expired
            if (chrono::system_clock::now() -
                    package_sending.last_attempt_time >
                GenericProtocolConstants::resend_timeout) {
                // Check if the message has remaining attempts
                if (package_sending.remaining_attempts > 0) {
                    package_sending.last_attempt_time =
                        chrono::system_clock::now();
                    package_sending.remaining_attempts--;
                    lock.unlock();
                    this->preprocessPackage(
                        package_sending.package,
                        GenericProtocolConstants::max_attempts_to_send_package -
                            package_sending.remaining_attempts);
                    lock.lock();
                } else {
                    // Finished attempts to send the message
                    if (GenericProtocolConstants::debug_information) {
                        this->printInformation(
                            "Package [" +
                                to_string(package_sending.package.getMessage()
                                              .getId()) +
                                "] has been removed from the network " +
                                this->getName() + "!",
                            cout, PrettyConsole::Color::RED);
                    }
                    it = this->unconfirmed_packages->erase(it);
                }
            } else {
                it++;
            }
        }

        // Wait for a message to send
        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(
            GenericProtocolConstants::interval_to_check_unconfirmed_packages));
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
            this->processPackage(package);
            lock.lock();
        } else {
            // Wait for a message to process
            this->package_processed_cv.wait(lock);
        }
    }
}

/* Static Methods */

unique_ptr<Network> Network::createNetwork(
    string name, shared_ptr<uuids::uuid_random_generator> uuid_generator,
    function<shared_ptr<Entity>(uuids::uuid)> get_entity_by_id) {
    return unique_ptr<Network>(
        new Network(name, uuid_generator, get_entity_by_id));
}
