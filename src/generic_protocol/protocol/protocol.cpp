#include "protocol.hpp"

using namespace std;

Protocol::Protocol(shared_ptr<uuids::uuid_random_generator> uuid_generator) {
    this->uuid_generator = uuid_generator;
    this->entities = make_shared<EntitiesList>();
}

Protocol::~Protocol() {}

uuids::uuid Protocol::createEntity(string name) {
    uuids::uuid entity_id = this->uuid_generator->operator()();
    return entity_id;
}