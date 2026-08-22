#include "../PriceLevel.hpp"
#include "../Ticket.hpp"
#include "../Ledger.hpp"
#include "../TicketFeed.hpp"
#include "../TicketGenerator.hpp"

#include <gtest/gtest.h>

struct ExampleTicketsTests: public ::testing::Test
{
    Ledger* ledger;
    TicketFeed* ticketFeed;
    TicketGenerator* ticketGenerator;

    virtual void SetUp() override{
        ledger = new Ledger();
        ticketFeed = new TicketFeed(ledger);
        ticketGenerator = new TicketGenerator(ledger);
    }

    virtual void TearDown() override{
        delete ticketGenerator;
        delete ticketFeed;
        delete ledger;
    }
};

TEST_F(ExampleTicketsTests, CreateInitialTicketsTest) {
    ticketGenerator->createInitialTickets(10000, 300);
}

TEST_F(ExampleTicketsTests, ProcessInitialTicketsTest) {
    ticketFeed->processTicketsFromFile("./initialOrders.txt");
}

TEST_F(ExampleTicketsTests, CreateTicketsTest) {
    ticketFeed->processTicketsFromFile("./initialOrders.txt");
    ticketGenerator->createTickets(100000);
}
