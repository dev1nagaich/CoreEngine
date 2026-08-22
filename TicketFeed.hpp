#ifndef TICKETFEED_HPP
#define TICKETFEED_HPP

#include <string>

class Ledger;

class TicketFeed {
private:
    Ledger *ledger;

public:
    explicit TicketFeed(Ledger *_ledger);

    void processTicketsFromFile(const std::string &path);
};

#endif
