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

#include "entity.hpp"
#include "generic_protocol_constants.hpp"
#include "package.hpp"

using namespace std;

class Network {
   private:
    struct PackageSending {
        Package package;
        int remaining_attempts;
        chrono::time_point<chrono::system_clock> last_attempt_time;

        PackageSending(Package package)
            : package(package),
              last_attempt_time(chrono::system_clock::now()),
              remaining_attempts(
                  GenericProtocolConstants::max_attempts_to_send_package - 1) {
        }  // Subtract 1 because the first attempt is done instantly, so the
           // sendingThreadJob should not execute another attempt for it
    };

    shared_ptr<uuids::uuid_random_generator> uuid_generator;
    string name;
    function<shared_ptr<Entity>(uuids::uuid)> get_entity_by_id;

    shared_ptr<map<uuids::uuid, PackageSending>> unconfirmed_packages;
    mutex unconfirmed_packages_mutex;

    thread package_sending_thread;
    int sending_packages_count;
    condition_variable package_sent_cv;  // Condition variable to notify when a
                                         // package has been sent
    bool can_stop_sending_thread;

    shared_ptr<queue<Package>> packages_to_process;
    mutex packages_to_process_mutex;

    thread processing_packages_thread;
    int processing_packages_count;
    condition_variable
        package_processed_cv;  // Condition variable to notify when a package
                               // has been processed
    bool can_stop_processing_thread;

    /* Construction */
    Network(string name,
            shared_ptr<uuids::uuid_random_generator> uuid_generator,
            function<shared_ptr<Entity>(uuids::uuid)> get_entity_by_id);

    /* Methods */
    shared_ptr<Entity> getEntityById(uuids::uuid entity_id);

    bool internalReceivePackage(Package package);
    void registerPackage(Package package);

    void sendingThreadJob();
    void joinSendingThread();
    void tryToConfirmSomePackage(
        optional<uuids::uuid> id_from_package_possibly_acknowledged);
    void removePackageFromUnconfirmedPackages(uuids::uuid package_id);

    bool preprocessPackage(Package package, int attempt = 1);
    bool hasPackageBeenLost(uuids::uuid message_id);
    bool insertPackageIntoProcessingQueue(Package package);

    void processingThreadJob();
    void processPackage(Package package);
    void simulateNetworkLatency();
    void simulatePacketCorruption(Package &package);
    void joinProcessingThread();

    void sendPackage(Package &package);
    void finishPackageProcessing();

    void printInformation(
        string information, ostream &output_stream,
        PrettyConsole::Color color = PrettyConsole::Color::DEFAULT) const;

   public:
    /* Destruction */
    ~Network();

    /* Getters */
    string getName() const;

    /* Methods */
    bool receivePackage(Package package);
    void joinThreads();

    /* Static Methods */
    static unique_ptr<Network> createNetwork(
        string name, shared_ptr<uuids::uuid_random_generator> uuid_generator,
        function<shared_ptr<Entity>(uuids::uuid)> get_entity_by_id);
};

#endif  // NETWORK_HPP_
