#ifndef PACKAGE_HPP_
#define PACKAGE_HPP_

#include "message.hpp"

using namespace std;

class Package {
   private:
    Message message;
    bool should_be_confirmed;
    unsigned int sequence_number;
    bool corrupted;

   public:
    /* Construction */
    Package(Message message, bool should_be_confirmed,
            unsigned int sequence_number, bool corrupted)
        : message(message),
          should_be_confirmed(should_be_confirmed),
          sequence_number(sequence_number),
          corrupted(corrupted) {}
    ~Package();

    /* Getters */
    Message getMessage() const;
    bool isCorrupted() const;

    /* Methods */
    void print(function<void(string)> print_message) const;
};

#endif  // PACKAGE_HPP_
