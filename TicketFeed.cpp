#include "TicketFeed.hpp"
#include "Ledger.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

TicketFeed::TicketFeed(Ledger *_ledger) : ledger(_ledger) {}

// Reads newline-delimited ticket commands from a file and replays them against the ledger.
// Supported lines:
//   L <ticketId> <isBuySide 0|1> <quantity> <price>   add limit ticket
//   C <ticketId>                                      cancel limit ticket
//   M <ticketId> <newQuantity> <newPrice>              modify limit ticket
void TicketFeed::processTicketsFromFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Could not open ticket file: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::istringstream iss(line);
        char command;
        iss >> command;

        if (command == 'L')
        {
            int ticketId, isBuySide, quantity, price;
            iss >> ticketId >> isBuySide >> quantity >> price;
            ledger->addLimitTicket(ticketId, isBuySide != 0, quantity, price);
        } else if (command == 'C')
        {
            int ticketId;
            iss >> ticketId;
            ledger->cancelLimitTicket(ticketId);
        } else if (command == 'M')
        {
            int ticketId, newQuantity, newPrice;
            iss >> ticketId >> newQuantity >> newPrice;
            ledger->modifyLimitTicket(ticketId, newQuantity, newPrice);
        }
    }
}
