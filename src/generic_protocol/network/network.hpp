#ifndef NETWORK_HPP_
#define NETWORK_HPP_

#include <uuid.h>

#include <condition_variable>
#include <map>
#include <memory>
#include <message.hpp>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#include "../entity/entity.hpp"
#include "generic_protocol_constants.hpp"

using namespace std;

class Network {
   private:
    struct MessageSending {
        Message message;
        int remaining_attempts;
        chrono::time_point<chrono::system_clock> last_attempt_time;

        MessageSending(Message message)
            : message(message),
              last_attempt_time(chrono::system_clock::now()),
              remaining_attempts(
                  GenericProtocolConstants::max_attempts_to_send_message - 1) {
        }  // Subtract 1 because the first attempt is done instantly, so the
           // sendingThreadJob should not execute another attempt for it
    };

    shared_ptr<uuids::uuid_random_generator> uuid_generator;
    string name;

    unordered_map<uuids::uuid, shared_ptr<Entity>> entities;
    mutex entities_mutex;

    map<uuids::uuid, MessageSending> unconfirmed_messages;
    mutex unconfirmed_messages_mutex;

    thread message_sending_thread;
    int sending_messages_count;
    condition_variable message_sent_cv;  // Condition variable to notify when a
                                         // message has been sent
    bool can_stop_sending_thread;

    queue<Package> packages_to_process;
    mutex packages_to_process_mutex;

    thread processing_packages_thread;
    int processing_packages_count;
    condition_variable
        package_processed_cv;  // Condition variable to notify when a message
                               // has been processed
    bool can_stop_processing_thread;

    /* Construction */
    Network(string name,
            shared_ptr<uuids::uuid_random_generator> uuid_generator);

    /* Methods */
    bool receivePackage(Package package);
    void registerPackage(Package package);

    void sendingThreadJob();
    void joinSendingThread();
    void tryToConfirmSomeMessage(
        optional<uuids::uuid> id_from_message_possibly_acknowledged);
    void removeMessageFromUnconfirmedMessages(uuids::uuid message_id);

    bool preprocessPackage(Package package, int attempt = 1);
    bool hasPackageBeenLost(uuids::uuid message_id);
    bool insertPackageIntoProcessingQueue(Package package);

    void processingThreadJob();
    void processMessage(Package package);
    void simulateNetworkLatency();
    void simulatePacketCorruption(Message &message);
    void joinProcessingThread();

    void sendMessage(Package package);
    void finishMessageProcessing();

    void printInformation(
        string information, ostream &output_stream,
        PrettyConsole::Color color = PrettyConsole::Color::DEFAULT) const;

   public:
    /* Destruction */
    ~Network();

    /* Getters */
    string getName() const;

    /* Methods */
    void connectEntity(shared_ptr<Entity> entity);
    void disconnectEntity(uuids::uuid entity_id);
    bool receiveMessage(Message message);
    void joinThreads();

    /* Static Methods */
    static unique_ptr<Network> createNetwork(
        string name, shared_ptr<uuids::uuid_random_generator> uuid_generator);
};

#endif  // NETWORK_HPP_
