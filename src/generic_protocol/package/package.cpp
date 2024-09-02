#include "package.hpp"

#include "util.hpp"

using namespace std;

Message Package::getMessage() const { return this->message; }

bool Package::isCorrupted() const { return this->corrupted; }

void Package::print(function<void(string)> print_message) const {
    this->message.print(print_message);
    print_message("Should be confirmed: " +
                  Util::getFormattedBool(this->should_be_confirmed));
    print_message("Sequence number: " + to_string(this->sequence_number));
    print_message("Corrupted: " + Util::getFormattedBool(this->corrupted));
}