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

    Protocol protocol(uuid_generator, "Zircônia");

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

    deque<string> data_fragments = {"Fragment 1",  "Fragment 2",  "Fragment 3",
                                    "Fragment 4",  "Fragment 5",  "Fragment 6",
                                    "Fragment 7",  "Fragment 8",  "Fragment 9",
                                    "Fragment 10", "Fragment 11", "Fragment 12",
                                    "Fragment 13", "Fragment 14", "Fragment 15",
                                    "Fragment 16", "Fragment 17", "Fragment 18",
                                    "Fragment 19", "Fragment 20"

    };

    output_stream << "Sending messages" << endl;
    protocol.sendData(entity_a, entity_b, data_fragments, output_stream);

    protocol.printEntitiesStorage(output_stream);
    output_stream << endl;
    cout << output_stream.str();
    output_stream.str("");

    cout << output_stream.str();
    output_stream.str("");
}
