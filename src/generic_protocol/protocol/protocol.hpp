#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <deque>
#include <memory>
#include <sstream>

#include "connection.hpp"
#include "entity.hpp"
#include "network.hpp"
#include "uuid.h"

using namespace std;

class Protocol {
   private:
    shared_ptr<uuids::uuid_random_generator> uuid_generator;
    shared_ptr<EntitiesList> entities;
    shared_ptr<ConnectionsMap> connections;
    unique_ptr<Network> network;

    shared_ptr<Entity> getEntityById(uuids::uuid entity_id);
    shared_ptr<Connection> connectEntities(shared_ptr<Entity> source_entity,
                                           shared_ptr<Entity> target_entity,
                                           ostringstream &output_stream);

    /* Static methods */
    static void printInformation(string information,
                                 ostringstream &output_stream);

   public:
    /* Construction */
    Protocol(shared_ptr<uuids::uuid_random_generator> uuid_generator,
             string network_name);
    ~Protocol();

    /* Methods */
    uuids::uuid createEntity(string name, ostringstream &output_stream);
    void sendData(uuids::uuid source_entity_id, uuids::uuid target_entity_id,
                  deque<string> contents, ostringstream &output_stream);

    /* Static methods */
    void printEntitiesStorage(ostringstream &output_stream);
};

#endif  // PROTOCOL_HPP_
