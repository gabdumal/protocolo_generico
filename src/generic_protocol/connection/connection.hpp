#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <map>

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
using ConnectionsMapPointer = shared_ptr<ConnectionsMap>;

class Connection {
   private:
    optional<uuids::uuid> syn_message_id;
    optional<uuids::uuid> ack_syn_message_id;
    optional<uuids::uuid> ack_ack_syn_message_id;
    optional<uuids::uuid> last_data_message_id;

   public:
    Connection()
        : syn_message_id(nullopt),
          ack_syn_message_id(nullopt),
          ack_ack_syn_message_id(nullopt),
          last_data_message_id(nullopt) {}
    ~Connection() {}

    void connect(uuids::uuid message_id, ConnectionStep step);
    void removeConnection();
    bool isConnectedAtStep(ConnectionStep step);
    bool canStoreData(uuids::uuid message_id);
    void setLastDataMessageId(uuids::uuid message_id);

    /* Static Methods */
    static void connect(ConnectionsMapPointer connections_ptr,
                        ConnectFunctionParameters connect_function_parameters);
    static void removeConnection(ConnectionsMapPointer connections_ptr,
                                 RemoveConnectionFunctionParameters
                                     remove_connection_function_parameters);
    static bool isConnectedAtStep(ConnectionsMapPointer connections_ptr,
                                  IsConnectedAtStepFunctionParameters
                                      is_connected_at_step_function_parameters);
    static bool canStoreData(
        ConnectionsMapPointer connections_ptr,
        CanStoreDataFunctionParameters can_store_data_function_parameters);
    static void setLastDataMessageId(ConnectionsMapPointer connections_ptr,
                                     SetLastDataMessageIdFunctionParameters
                                         set_last_data_message_id_parameters);
};

#endif  // CONNECTION_HPP_
