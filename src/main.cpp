#include <generic_protocol.hpp>
#include <uuid.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    cout << "DCC042 - Computer Networks" << endl
         << endl;

    // Random UUID generator
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator uuidGenerator{generator};

    GenericProtocol genericProtocol(&uuidGenerator);
    genericProtocol.run();

    return 0;
}