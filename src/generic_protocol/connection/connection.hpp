#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <map>
#include <queue>

#include "entity.hpp"

using namespace std;

class Connection;

struct UuidPairComparator {
    bool operator()(const pair<uuids::uuid, uuids::uuid>& lhs,
                    const pair<uuids::uuid, uuids::uuid>& rhs) const {
        // Compare the pairs regardless of the order of UUIDs
        return (lhs.first < rhs.first && lhs.second < rhs.second) ||
               (lhs.first < rhs.second && lhs.second < rhs.first);
    }
};

using ConnectionsMap = map<pair<uuids::uuid, uuids::uuid>,
                           shared_ptr<Connection>, UuidPairComparator>;

class Connection {
   private:
    optional<uuids::uuid> syn_message_id;
    optional<uuids::uuid> ack_syn_message_id;
    optional<uuids::uuid> ack_ack_syn_message_id;

    mutex queue_mutex;
    unsigned int buffer_size;
    shared_ptr<queue<uuids::uuid>> unconfirmed_sent_packages;

   public:
    /* Construction */
    Connection(unsigned int buffer_size)
        : syn_message_id(nullopt),
          ack_syn_message_id(nullopt),
          ack_ack_syn_message_id(nullopt),
          buffer_size(buffer_size),
          unconfirmed_sent_packages(make_shared<queue<uuids::uuid>>()) {}
    ~Connection() {}

    /* Getters */
    bool isConnectedAtStep(ConnectionStep step);
    bool canSendPackage();
    bool canStoreData(uuids::uuid message_id);

    /* Setters */
    void setLastDataMessageId(uuids::uuid message_id);

    /* Methods */
    void connect(uuids::uuid message_id, ConnectionStep step);
    void removeConnection();
    void enqueuePackage(uuids::uuid message_id);
    void dequeuePackage(uuids::uuid message_id);
    void lockQueue();
    void unlockQueue();

    /* Static Methods */
    static void connect(shared_ptr<ConnectionsMap> connections_ptr,
                        ConnectFunctionParameters connect_function_parameters);
    static void removeConnection(shared_ptr<ConnectionsMap> connections_ptr,
                                 RemoveConnectionFunctionParameters
                                     remove_connection_function_parameters);
    static bool isConnectedAtStep(shared_ptr<ConnectionsMap> connections_ptr,
                                  IsConnectedAtStepFunctionParameters
                                      is_connected_at_step_function_parameters);
    static bool canSendPackage(
        shared_ptr<ConnectionsMap> connections_ptr,
        CanSendPackageFunctionParameters can_send_package_function_parameters);
    static bool canStoreData(
        shared_ptr<ConnectionsMap> connections_ptr,
        CanStoreDataFunctionParameters can_store_data_function_parameters);
    static void enqueuePackage(
        shared_ptr<ConnectionsMap> connections_ptr,
        EnqueuePackageFunctionParameters enqueue_package_function_parameters);
    static void dequeuePackage(
        shared_ptr<ConnectionsMap> connections_ptr,
        DequeuePackageFunctionParameters dequeue_package_function_parameters);
};

#endif  // CONNECTION_HPP_
