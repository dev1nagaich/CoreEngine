#ifndef TICKETGENERATOR_HPP
#define TICKETGENERATOR_HPP

#include <random>

class Ledger;

class TicketGenerator {
private:
    Ledger *ledger;
    std::mt19937 gen;
    int nextTicketId;

public:
    explicit TicketGenerator(Ledger *_ledger);

    // Writes a file of resting limit tickets spread across a price range,
    // then replays them into the ledger.
    void createInitialTickets(int count, int priceRange);

    // Generates a stream of random tickets (limit/cancel/modify) directly
    // against the ledger, exercising the matching and cancel paths.
    void createTickets(int count);
};

#endif
