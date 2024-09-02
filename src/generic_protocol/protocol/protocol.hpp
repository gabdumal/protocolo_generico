#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <deque>
#include <sstream>

#include "connection.hpp"
#include "entity.hpp"
#include "uuid.h"

using namespace std;

class Protocol {
   private:
    shared_ptr<uuids::uuid_random_generator> uuid_generator;
    shared_ptr<EntitiesList> entities;
    // unique_ptr<Network> network;
    shared_ptr<ConnectionsMap> connections;

    static void printMessage(string message, ostringstream &output_stream);

   public:
    /* Construction */
    Protocol(shared_ptr<uuids::uuid_random_generator> uuid_generator);
    ~Protocol();

    /* Methods */
    uuids::uuid createEntity(string name, ostringstream &output_stream);
    void sendData(uuids::uuid source_entity_id, uuids::uuid target_entity_id,
                  deque<string> contents);
};

#endif  // PROTOCOL_HPP_
