#include "PriceLevel.hpp"
#include "Ticket.hpp"
#include <iostream>

PriceLevel::PriceLevel(int _price, bool _isBuySide, int _ticketCount, int _aggregateQty)
    : price(_price), isBuySide(_isBuySide), ticketCount(_ticketCount), aggregateQty(_aggregateQty),
    parent(nullptr), leftChild(nullptr), rightChild(nullptr),
    frontTicket(nullptr), backTicket(nullptr) {}

PriceLevel::~PriceLevel()
{
    if (parent != nullptr) {
        bool leftOrRightChild = (price < parent->getPrice());
        // Case 1: Node with only one child or no child
        if (leftChild == nullptr) {
            if (leftOrRightChild) {
                parent->leftChild = rightChild;
            } else {
                parent->rightChild = rightChild;
            }
            if (rightChild != nullptr) {
                rightChild->setParent(parent);
            }
            return;
        } else if (rightChild == nullptr) {
            if (leftOrRightChild) {
                parent->leftChild = leftChild;
            } else {
                parent->rightChild = leftChild;
            }
            leftChild->setParent(parent);
            return;
        }

        // Case 2: Node with two children
        PriceLevel* temp = rightChild;

        while (temp->getLeftChild() != nullptr)
        {
            temp = temp->getLeftChild();
        }

        if (rightChild->getLeftChild() != nullptr)
        {
            temp->getParent()->setLeftChild(temp->getRightChild());
            if (temp->getRightChild() != nullptr)
            {
                temp->getRightChild()->setParent(temp->getParent());
            }
            temp->setRightChild(rightChild);
            rightChild->setParent(temp);
        }

        temp->setParent(parent);
        temp->setLeftChild(leftChild);
        leftChild->setParent(temp);
        if (leftOrRightChild) {
            parent->leftChild = temp;
        } else {
            parent->rightChild = temp;
        }
    } else
    {
        // Case 1: Node with only one child or no child
        if (leftChild == nullptr && rightChild == nullptr) {
            return;
        } else if (leftChild == nullptr)
        {
            rightChild->setParent(nullptr);
            return;
        } else if (rightChild == nullptr)
        {
            leftChild->setParent(nullptr);
            return;
        }

        // Case 2: Node with two children
        PriceLevel* temp = rightChild;
        while (temp->getLeftChild() != nullptr) {
            temp = temp->getLeftChild();
        }
        if (rightChild->getLeftChild() != nullptr)
        {
            temp->getParent()->setLeftChild(temp->getRightChild());
            if (temp->getRightChild() != nullptr)
            {
                temp->getRightChild()->setParent(temp->getParent());
            }
            temp->setRightChild(rightChild);
            rightChild->setParent(temp);
        }
        temp->setParent(parent); // nullptr
        temp->setLeftChild(leftChild);
        leftChild->setParent(temp);
    }
}

Ticket* PriceLevel::getFrontTicket() const
{
    return frontTicket;
}

int PriceLevel::getPrice() const
{
    return price;
}

int PriceLevel::getTicketCount() const
{
    return ticketCount;
}

int PriceLevel::getAggregateQty() const
{
    return aggregateQty;
}

bool PriceLevel::getIsBuySide() const
{
    return isBuySide;
}

PriceLevel* PriceLevel::getParent() const
{
    return parent;
}

PriceLevel* PriceLevel::getLeftChild() const
{
    return leftChild;
}

PriceLevel* PriceLevel::getRightChild() const
{
    return rightChild;
}

void PriceLevel::setParent(PriceLevel* newParent)
{
    parent = newParent;
}

void PriceLevel::setLeftChild(PriceLevel* newLeftChild)
{
    leftChild = newLeftChild;
}

void PriceLevel::setRightChild(PriceLevel* newRightChild)
{
    rightChild = newRightChild;
}

void PriceLevel::partiallyFillAggregateQty(int filledQty)
{
    aggregateQty -= filledQty;
}

// Add a ticket to the price level
void PriceLevel::append(Ticket *ticket)
{
        if (frontTicket == nullptr) {
            frontTicket = backTicket = ticket;
        } else {
            backTicket->nextTicket = ticket;
            ticket->prevTicket = backTicket;
            ticket->nextTicket = nullptr;
            backTicket = ticket;
        }
        ticketCount += 1;
        aggregateQty += ticket->getQuantity();
        ticket->ownerLevel = this;
}

void PriceLevel::printForward() const
{
    Ticket* current = frontTicket;
    while (current != nullptr) {
        std::cout << current->getTicketId() << " ";
        current = current->nextTicket;
    }
    std::cout << std::endl;
}

void PriceLevel::printBackward() const
{
    Ticket* current = backTicket;
    while (current != nullptr) {
        std::cout << current->getTicketId() << " ";
        current = current->prevTicket;
    }
    std::cout << std::endl;
}

void PriceLevel::print() const
{
    std::cout << "Price: " << price
    << ", Aggregate Quantity: " << aggregateQty
    << ", Ticket Count: " << ticketCount
    << std::endl;
}
