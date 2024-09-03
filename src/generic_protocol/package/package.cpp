#include "package.hpp"

#include "util.hpp"

using namespace std;

/* Getters */

Message Package::getMessage() const { return this->message; }

bool Package::isCorrupted() const { return this->is_corrupted; }

bool Package::shouldBeConfirmed() const { return this->should_be_confirmed; }

/* Setters */

void Package::setCorrupted(bool is_corrupted) {
    this->is_corrupted = is_corrupted;
}

void Package::setIdFromMessageBeingAcknowledged(uuids::uuid id_from_message) {
    this->message.setIdFromMessageBeingAcknowledged(id_from_message);
}

/* Methods */

void Package::print(function<void(string)> print_message) const {
    this->message.print(print_message);
    print_message("Should be confirmed: " +
                  Util::getFormattedBool(this->should_be_confirmed));
    print_message("Sequence number: " + to_string(this->sequence_number));
    print_message("Corrupted: " + Util::getFormattedBool(this->is_corrupted));
}