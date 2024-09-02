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
    output_stream.str("");

    Protocol protocol(uuid_generator);

    output_stream << "Creating entities" << endl;
    uuids::uuid entity_a = protocol.createEntity("Aroeira", output_stream);
    uuids::uuid entity_b = protocol.createEntity("Baobá", output_stream);
    output_stream << endl;
    cout << output_stream.str();
    output_stream.str("");

    protocol.printEntitiesStorage(output_stream);

    output_stream << endl;
    cout << output_stream.str();
    output_stream.str("");
}
