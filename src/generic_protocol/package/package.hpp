#ifndef PACKAGE_HPP_
#define PACKAGE_HPP_

#include "message.hpp"

using namespace std;

class Package {
   private:
    Message message;
    bool should_be_confirmed;
    unsigned int sequence_number;
    bool is_corrupted;

   public:
    /* Construction */
    Package(Message message, bool should_be_confirmed,
            unsigned int sequence_number)
        : message(message),
          should_be_confirmed(should_be_confirmed),
          sequence_number(sequence_number),
          is_corrupted(false) {}

    Package(Message message, bool should_be_confirmed)
        : Package(message, should_be_confirmed, 0) {}

    ~Package();

    /* Getters */
    Message getMessage() const;
    bool isCorrupted() const;
    bool shouldBeConfirmed() const;

    /* Setters */
    void setCorrupted(bool is_corrupted);
    void setIdFromMessageBeingAcknowledged(uuids::uuid id_from_message);

    /* Methods */
    void print(function<void(string)> print_message) const;
};

#endif  // PACKAGE_HPP_
