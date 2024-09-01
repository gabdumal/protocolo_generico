#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <map>
// #include <memory>

#include "entity.hpp"

using namespace std;

class Connection {
   private:
    optional<uuids::uuid> syn_message_id;
    optional<uuids::uuid> ack_syn_message_id;
    optional<uuids::uuid> ack_ack_syn_message_id;

   public:
    // struct Wrapper {
    //     pair<shared_ptr<Entity>, shared_ptr<Entity>> key;
    //     shared_ptr<Connection> connection;

    //     Wrapper(pair<shared_ptr<Entity>, shared_ptr<Entity>> key,
    //             shared_ptr<Connection> connection)
    //         : key(key), connection(connection) {}
    // };

    Connection()
        : syn_message_id(nullopt),
          ack_syn_message_id(nullopt),
          ack_ack_syn_message_id(nullopt) {}
    ~Connection() {}

    bool isConnectedAtStep(ConnectionStep step);
    void connect(uuids::uuid message_id, ConnectionStep step);
    void removeConnection();

    /* Static Methods */
    static bool isConnectedAtStep(
        map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
        uuids::uuid source_entity_id, uuids::uuid target_entity_id,
        ConnectionStep step);
    static void connect(
        map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
        uuids::uuid source_entity_id, uuids::uuid target_entity_id,
        uuids::uuid message_id, ConnectionStep step);
    static void removeConnection(
        map<pair<uuids::uuid, uuids::uuid>, shared_ptr<Connection>> connections,
        uuids::uuid source_entity_id, uuids::uuid target_entity_id);
};

#endif  // CONNECTION_HPP_
