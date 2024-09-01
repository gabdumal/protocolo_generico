#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <map>
#include <memory>

#include "entity.hpp"

using namespace std;

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
    static bool isConnectedAtStep(
        map<pair<shared_ptr<Entity>, shared_ptr<Entity>>, Connection>
            &connections,
        shared_ptr<Entity> source, shared_ptr<Entity> target,
        ConnectionStep step);
    static void connect(map<pair<shared_ptr<Entity>, shared_ptr<Entity>>,
                            Connection> &connections,
                        shared_ptr<Entity> source, shared_ptr<Entity> target,
                        uuids::uuid message_id, ConnectionStep step);
    static void removeConnection(
        map<pair<shared_ptr<Entity>, shared_ptr<Entity>>, Connection>
            &connections,
        shared_ptr<Entity> source, shared_ptr<Entity> target);
};

#endif  // CONNECTION_HPP_
