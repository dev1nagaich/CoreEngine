#include "TicketGenerator.hpp"
#include "Ledger.hpp"
#include "PriceLevel.hpp"
#include "Ticket.hpp"
#include <fstream>
#include <sstream>
#include <random>

TicketGenerator::TicketGenerator(Ledger *_ledger)
    : ledger(_ledger), gen(std::random_device{}()), nextTicketId(1) {}

// Writes `count` resting limit tickets spread across [1, priceRange] to
// initialOrders.txt, split roughly evenly between buy and sell, then
// replays that file into the ledger.
void TicketGenerator::createInitialTickets(int count, int priceRange)
{
    std::uniform_int_distribution<> priceDist(1, priceRange);
    std::uniform_int_distribution<> quantityDist(1, 500);
    std::uniform_int_distribution<> sideDist(0, 1);

    std::ofstream file("initialOrders.txt");
    for (int i = 0; i < count; ++i)
    {
        int ticketId = nextTicketId++;
        int isBuySide = sideDist(gen);
        int quantity = quantityDist(gen);
        int price = priceDist(gen);
        file << "L " << ticketId << " " << isBuySide << " " << quantity << " " << price << "\n";
    }
    file.close();

    std::ifstream in("initialOrders.txt");
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        char command;
        int ticketId, isBuySide, quantity, price;
        std::istringstream iss(line);
        iss >> command >> ticketId >> isBuySide >> quantity >> price;
        ledger->addLimitTicket(ticketId, isBuySide != 0, quantity, price);
    }
}

// Fires `count` random limit tickets directly at the ledger, priced close
// to the current best bid/ask so a meaningful fraction cross and execute.
void TicketGenerator::createTickets(int count)
{
    std::uniform_int_distribution<> quantityDist(1, 500);
    std::uniform_int_distribution<> sideDist(0, 1);
    std::uniform_int_distribution<> spreadDist(-10, 10);

    for (int i = 0; i < count; ++i)
    {
        int ticketId = nextTicketId++;
        bool isBuySide = sideDist(gen) != 0;
        int quantity = quantityDist(gen);

        int basePrice = 500;
        PriceLevel* bestBid = ledger->getBestBid();
        PriceLevel* bestAsk = ledger->getBestAsk();
        if (isBuySide && bestAsk != nullptr)
        {
            basePrice = bestAsk->getPrice();
        } else if (!isBuySide && bestBid != nullptr)
        {
            basePrice = bestBid->getPrice();
        }

        int price = basePrice + spreadDist(gen);
        if (price < 1) price = 1;

        ledger->addLimitTicket(ticketId, isBuySide, quantity, price);
    }
}
