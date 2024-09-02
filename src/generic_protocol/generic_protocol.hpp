#ifndef _GENERIC_PROTOCOL_HPP
#define _GENERIC_PROTOCOL_HPP

#include <uuid.h>

#include <memory>

using namespace std;

class GenericProtocol {
   public:
    static void run(shared_ptr<uuids::uuid_random_generator> uuid_generator);
};

#endif  // _GENERIC_PROTOCOL_HPP
