#include "generic_protocol.hpp"

#include <iostream>
#include <memory>
#include <pretty_console.hpp>
#include <sstream>

#include "protocol.hpp"
#include "uuid.h"

using namespace std;

void GenericProtocol::run(
    shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    ostringstream output_stream;

    output_stream << "01. GENERIC PROTOCOL" << endl << endl;
    cout << output_stream.str();

    Protocol protocol(uuid_generator);

    uuids::uuid entity_a = protocol.createEntity("Aroeira");
    uuids::uuid entity_b = protocol.createEntity("Baobá");
}