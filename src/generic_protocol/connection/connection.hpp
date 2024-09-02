#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <map>

#include "entity.hpp"

using namespace std;

class Connection;

using ConnectionsMap =
    map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>>;
using ConnectionsMapPointer = shared_ptr<ConnectionsMap>;

class Connection {
   private:
    optional<uuids::uuid> syn_message_id;
    optional<uuids::uuid> ack_syn_message_id;
    optional<uuids::uuid> ack_ack_syn_message_id;

   public:
    Connection()
        : syn_message_id(nullopt),
          ack_syn_message_id(nullopt),
          ack_ack_syn_message_id(nullopt) {}
    ~Connection() {}

    bool isConnectedAtStep(ConnectionStep step);
    void connect(uuids::uuid message_id, ConnectionStep step);
    void removeConnection();

    /* Static Methods */
    static void connect(ConnectionsMapPointer connections_ptr,
                        ConnectFunctionParameters connect_function_parameters);
    static void removeConnection(ConnectionsMapPointer connections_ptr,
                                 RemoveConnectionFunctionParameters
                                     remove_connection_function_parameters);
    static bool isConnectedAtStep(ConnectionsMapPointer connections_ptr,
                                  IsConnectedAtStepFunctionParameters
                                      is_connected_at_step_function_parameters);
};

#endif  // CONNECTION_HPP_
