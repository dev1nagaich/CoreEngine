#include "TicketGenerator.hpp"
#include "TicketFeed.hpp"
#include "Ledger.hpp"
#include "PriceLevel.hpp"
#include "Ticket.hpp"
#include <iostream>
#include <vector>
#include <chrono>

int main() {
    Ledger* ledger = new Ledger();

    TicketFeed ticketFeed(ledger);

    // TicketGenerator ticketGenerator(ledger);

    // ticketGenerator.createInitialTickets(10000, 300);

    ticketFeed.processTicketsFromFile("./initialOrders.txt");

    // ticketGenerator.createTickets(5000000);


    // Start measuring time
    auto start = std::chrono::high_resolution_clock::now();

    ticketFeed.processTicketsFromFile("./Orders.txt");

    // Stop measuring time
    auto stop = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "Time taken to process tickets: " << duration.count() << " milliseconds" << std::endl;

    delete ledger;
    return 0;
}
