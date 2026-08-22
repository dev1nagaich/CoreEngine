#include "Ticket.hpp"
#include "PriceLevel.hpp"
#include <iostream>

Ticket::Ticket(int _ticketId, bool _isBuySide, int _quantity, int _priceRef)
    : ticketId(_ticketId), isBuySide(_isBuySide), quantity(_quantity), priceRef(_priceRef),
    nextTicket(nullptr), prevTicket(nullptr), ownerLevel(nullptr) {}

int Ticket::getQuantity() const
{
    return quantity;
}

int Ticket::getTicketId() const
{
    return ticketId;
}

bool Ticket::getIsBuySide() const
{
    return isBuySide;
}

int Ticket::getPriceRef() const
{
    return priceRef;
}

PriceLevel* Ticket::getOwnerLevel() const
{
    return ownerLevel;
}

void Ticket::partiallyFillTicket(int filledQty)
{
    quantity -= filledQty;
    ownerLevel->partiallyFillAggregateQty(filledQty);
}

// Remove ticket from its owning price level
void Ticket::cancel()
{
    if (prevTicket == nullptr)
    {
        ownerLevel->frontTicket = nextTicket;
    } else
    {
        prevTicket->nextTicket = nextTicket;
    }
    if (nextTicket == nullptr)
    {
        ownerLevel->backTicket = prevTicket;
    } else
    {
        nextTicket->prevTicket = prevTicket;
    }

    ownerLevel->aggregateQty -= quantity;
    ownerLevel->ticketCount -= 1;
}

// Execute front ticket
void Ticket::execute()
{
    ownerLevel->frontTicket = nextTicket;
    if (nextTicket == nullptr)
    {
        ownerLevel->backTicket = nullptr;
    } else
    {
        nextTicket->prevTicket = nullptr;
    }
    nextTicket = nullptr;
    prevTicket = nullptr;

    ownerLevel->aggregateQty -= quantity;
    ownerLevel->ticketCount -= 1;
}

void Ticket::modifyTicket(int newQuantity, int newPriceRef)
{
    quantity = newQuantity;
    priceRef = newPriceRef;
    nextTicket = nullptr;
    prevTicket = nullptr;
    ownerLevel = nullptr;
}

void Ticket::setQuantity(int newQuantity)
{
    quantity = newQuantity;
}

void Ticket::print() const
{
    std::cout << "Ticket ID: " << ticketId
    << ", Ticket Side: " << (isBuySide == 1 ? "buy" : "sell")
    << ", Ticket Quantity: " << quantity
    << ", Ticket Price: " << priceRef
    << std::endl;
}
