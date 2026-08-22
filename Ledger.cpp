#include "Ledger.hpp"
#include "Ticket.hpp"
#include "PriceLevel.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <iterator>

Ledger::Ledger() : bidTree(nullptr), askTree(nullptr), bestAsk(nullptr), bestBid(nullptr),
            stopBidTree(nullptr), stopAskTree(nullptr), highestStopAsk(nullptr), lowestStopBid(nullptr){}

// When deleting the ledger need to ensure all used memory is freed
Ledger::~Ledger()
{
    for (auto& [id, ticket] : ticketRegistry) {
        delete ticket;
    }
    ticketRegistry.clear();

    for (auto& [price, level] : bidLevels) {
        delete level;
    }
    bidLevels.clear();

    for (auto& [price, level] : askLevels) {
        delete level;
    }
    askLevels.clear();

    for (auto& [stopPrice, stopLevel] : stopLevels) {
        delete stopLevel;
    }
    stopLevels.clear();
}

PriceLevel* Ledger::getBidTree() const
{
    return bidTree;
}

PriceLevel* Ledger::getAskTree() const
{
    return askTree;
}

PriceLevel* Ledger::getBestAsk() const
{
    return bestAsk;
}

PriceLevel* Ledger::getBestBid() const
{
    return bestBid;
}

PriceLevel* Ledger::getStopBidTree() const
{
    return stopBidTree;
}

PriceLevel* Ledger::getStopAskTree() const
{
    return stopAskTree;
}

PriceLevel* Ledger::getHighestStopAsk() const
{
    return highestStopAsk;
}

PriceLevel* Ledger::getLowestStopBid() const
{
    return lowestStopBid;
}

// Execute a market ticket
void Ledger::marketTicket(int ticketId, bool isBuySide, int quantity)
{
    fillCount = 0;
    rebalanceCount = 0;
    marketTicketHelper(ticketId, isBuySide, quantity);

    executeStopTickets(isBuySide);
}

// Add a new limit ticket to the ledger
void Ledger::addLimitTicket(int ticketId, bool isBuySide, int quantity, int price)
{
    rebalanceCount = 0;
    // Account for ticket being executed immediately
    quantity = limitTicketAsMarketTicket(ticketId, isBuySide, quantity, price);

    if (quantity != 0)
    {
        Ticket* newTicket = new Ticket(ticketId, isBuySide, quantity, price);
        ticketRegistry.emplace(ticketId, newTicket);

        auto& levelMap = isBuySide ? bidLevels : askLevels;

        if (levelMap.find(price) == levelMap.end())
        {
            addPriceLevel(price, newTicket->getIsBuySide());
        }
        levelMap.at(price)->append(newTicket);
        // limitTickets.insert(newTicket);
    } else {
        executeStopTickets(isBuySide);
    }
}

// Delete a limit ticket from the ledger
void Ledger::cancelLimitTicket(int ticketId)
{
    fillCount = 0;
    rebalanceCount = 0;
    Ticket* ticket = searchTicketRegistry(ticketId);

    if (ticket != nullptr)
    {
        ticket->cancel();
            if (ticket->getOwnerLevel()->getTicketCount() == 0)
            {
                deletePriceLevel(ticket->getOwnerLevel());
            }
        deleteFromTicketRegistry(ticketId);
        // limitTickets.erase(ticket);
        delete ticket;
    }
}

// Modify an existing limit ticket
void Ledger::modifyLimitTicket(int ticketId, int newQuantity, int newPrice)
{
    fillCount = 0;
    rebalanceCount = 0;
    Ticket* ticket = searchTicketRegistry(ticketId);
    if (ticket != nullptr)
    {
        ticket->cancel();
            if (ticket->getOwnerLevel()->getTicketCount() == 0)
            {
                deletePriceLevel(ticket->getOwnerLevel());
            }

        ticket->modifyTicket(newQuantity, newPrice);
        auto& levelMap = ticket->getIsBuySide() ? bidLevels : askLevels;

        if (levelMap.find(newPrice) == levelMap.end())
        {
            addPriceLevel(newPrice, ticket->getIsBuySide());
        }
        levelMap.at(newPrice)->append(ticket);
    }
}

// Add a stop ticket
void Ledger::addStopTicket(int ticketId, bool isBuySide, int quantity, int stopPrice)
{
    fillCount = 0;
    rebalanceCount = 0;
    // Account for stop ticket being executed immediately
    quantity = stopTicketAsMarketTicket(ticketId, isBuySide, quantity, stopPrice);

    if (quantity != 0)
    {
        Ticket* newTicket = new Ticket(ticketId, isBuySide, quantity, 0);
        ticketRegistry.emplace(ticketId, newTicket);

        if (stopLevels.find(stopPrice) == stopLevels.end())
        {
            addStopLevel(stopPrice, newTicket->getIsBuySide());
        }
        stopLevels.at(stopPrice)->append(newTicket);
        // stopTickets.insert(newTicket);
    }
}

// Delete a stop ticket from the stop ledger
void Ledger::cancelStopTicket(int ticketId)
{
    fillCount = 0;
    rebalanceCount = 0;
    Ticket* ticket = searchTicketRegistry(ticketId);

    if (ticket != nullptr)
    {
        ticket->cancel();
            if (ticket->getOwnerLevel()->getTicketCount() == 0)
            {
                deleteStopLevel(ticket->getOwnerLevel());
            }
        deleteFromTicketRegistry(ticketId);
        // stopTickets.erase(ticket);
        delete ticket;
    }
}

// Modify an existing stop ticket
void Ledger::modifyStopTicket(int ticketId, int newQuantity, int newStopPrice)
{
    fillCount = 0;
    rebalanceCount = 0;
    Ticket* ticket = searchTicketRegistry(ticketId);
    if (ticket != nullptr)
    {
        ticket->cancel();
            if (ticket->getOwnerLevel()->getTicketCount() == 0)
            {
                deleteStopLevel(ticket->getOwnerLevel());
            }

        ticket->modifyTicket(newQuantity, 0);

        if (stopLevels.find(newStopPrice) == stopLevels.end())
        {
            addStopLevel(newStopPrice, ticket->getIsBuySide());
        }
        stopLevels.at(newStopPrice)->append(ticket);
    }
}

// Add a stop limit ticket
void Ledger::addStopLimitTicket(int ticketId, bool isBuySide, int quantity, int price, int stopPrice)
{
    fillCount = 0;
    rebalanceCount = 0;
    // Account for stop limit ticket being executed immediately
    quantity = stopLimitTicketAsLimitTicket(ticketId, isBuySide, quantity, price, stopPrice);

    if (quantity != 0)
    {
        Ticket* newTicket = new Ticket(ticketId, isBuySide, quantity, price);
        ticketRegistry.emplace(ticketId, newTicket);

        if (stopLevels.find(stopPrice) == stopLevels.end())
        {
            addStopLevel(stopPrice, newTicket->getIsBuySide());
        }
        stopLevels.at(stopPrice)->append(newTicket);
        // stopLimitTickets.insert(newTicket);
    }
}

void Ledger::cancelStopLimitTicket(int ticketId)
{
    fillCount = 0;
    rebalanceCount = 0;
    Ticket* ticket = searchTicketRegistry(ticketId);

    if (ticket != nullptr)
    {
        ticket->cancel();
            if (ticket->getOwnerLevel()->getTicketCount() == 0)
            {
                deleteStopLevel(ticket->getOwnerLevel());
            }
        deleteFromTicketRegistry(ticketId);
        // stopLimitTickets.erase(ticket);
        delete ticket;
    }
}

// Modify an existing stop limit ticket
void Ledger::modifyStopLimitTicket(int ticketId, int newQuantity, int newPrice, int newStopPrice)
{
    fillCount = 0;
    rebalanceCount = 0;
    Ticket* ticket = searchTicketRegistry(ticketId);
    if (ticket != nullptr)
    {
        ticket->cancel();
            if (ticket->getOwnerLevel()->getTicketCount() == 0)
            {
                deleteStopLevel(ticket->getOwnerLevel());
            }

        ticket->modifyTicket(newQuantity, newPrice);

        if (stopLevels.find(newStopPrice) == stopLevels.end())
        {
            addStopLevel(newStopPrice, ticket->getIsBuySide());
        }
        stopLevels.at(newStopPrice)->append(ticket);
    }
}

// Get the height of a price level in a binary tree
int Ledger::getLevelHeight(PriceLevel* level) const {
    if (level == nullptr) {
        return 0; // Height of an empty tree is 0
    } else {
        int l_height = getLevelHeight(level->getLeftChild());
        int r_height = getLevelHeight(level->getRightChild());
        int max = std::max(l_height, r_height) + 1;
        return max;
    }
}

// Search the ticket registry to find a ticket
Ticket* Ledger::searchTicketRegistry(int ticketId) const
{
    auto it = ticketRegistry.find(ticketId);
    if (it != ticketRegistry.end())
    {
        return it->second;
    } else
    {
        std::cout << "No ticket number " << ticketId << std::endl;
        return nullptr;
    }
}

// Search the level maps to find a price level
PriceLevel* Ledger::searchLevelMaps(int price, bool isBuySide) const
{
    auto& levelMap = isBuySide ? bidLevels : askLevels;

    auto it = levelMap.find(price);
    if (it != levelMap.end())
    {
        return it->second;
    } else
    {
        std::cout << "No "<< (isBuySide ? "bid " : "ask ") << "level at " << price << std::endl;
        return nullptr;
    }
}

// Search the stop levels to find a stop level
PriceLevel* Ledger::searchStopLevels(int stopPrice) const
{
    auto it = stopLevels.find(stopPrice);
    if (it != stopLevels.end())
    {
        return it->second;
    } else
    {
        std::cout << "No stop level at " << stopPrice << std::endl;
        return nullptr;
    }
}

void Ledger::printPriceLevel(int price, bool isBuySide) const
{
    searchLevelMaps(price, isBuySide)->print();
}

void Ledger::printTicket(int ticketId) const
{
    searchTicketRegistry(ticketId)->print();
}

void Ledger::printLedgerEdges() const
{
    std::cout << "Bid edge: " << bestBid->getPrice()
    << "Ask edge: " << bestAsk->getPrice() << std::endl;
}

// Print out all the price and stop levels and their liquidity
void Ledger::printLedger() const
{
    std::vector<int> vec = inOrderTreeTraversal(getStopBidTree());
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << "-" << searchStopLevels(vec[i])->getAggregateQty();
        if (i != 0 && i != vec.size()-1 && vec[i] < vec[i-1]) {
            throw std::runtime_error("Error: vector is error");
        }
        if (i != vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    vec = inOrderTreeTraversal(getStopAskTree());
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << "-" << searchStopLevels(vec[i])->getAggregateQty();
        if (i != 0 && i != vec.size()-1 && vec[i] < vec[i-1]) {
            throw std::runtime_error("Error: Vector is error");
        }
        if (i != vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    vec = inOrderTreeTraversal(getBidTree());
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << "-" << searchLevelMaps(vec[i], true)->getAggregateQty();
        if (i != 0 && i != vec.size()-1 && vec[i] < vec[i-1]) {
            throw std::runtime_error("Error: vector is error");
        }
        if (i != vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    vec = inOrderTreeTraversal(getAskTree());
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << "-" << searchLevelMaps(vec[i], false)->getAggregateQty();
        if (i != 0 && i != vec.size()-1 && vec[i] < vec[i-1]) {
            throw std::runtime_error("Error: Vector is error");
        }
        if (i != vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

// In order traversal of the binary search tree
std::vector<int> Ledger::inOrderTreeTraversal(PriceLevel* root) const
{
    std::vector<int> result;
    if (root == nullptr)
        return result;

    std::vector<int> leftSubtree = inOrderTreeTraversal(root->getLeftChild());
    result.insert(result.end(), leftSubtree.begin(), leftSubtree.end());

    result.push_back(root->getPrice());

    std::vector<int> rightSubtree = inOrderTreeTraversal(root->getRightChild());
    result.insert(result.end(), rightSubtree.begin(), rightSubtree.end());

    return result;
}

// Pre order traversal of the binary search tree
std::vector<int> Ledger::preOrderTreeTraversal(PriceLevel* root) const
{
    std::vector<int> result;
    if (root == nullptr)
        return result;

    result.push_back(root->getPrice());

    std::vector<int> leftSubtree = preOrderTreeTraversal(root->getLeftChild());
    result.insert(result.end(), leftSubtree.begin(), leftSubtree.end());

    std::vector<int> rightSubtree = preOrderTreeTraversal(root->getRightChild());
    result.insert(result.end(), rightSubtree.begin(), rightSubtree.end());

    return result;
}

// Post order traversal of the binary search tree
std::vector<int> Ledger::postOrderTreeTraversal(PriceLevel* root) const
{
    std::vector<int> result;
    if (root == nullptr)
        return result;

    std::vector<int> leftSubtree = postOrderTreeTraversal(root->getLeftChild());
    result.insert(result.end(), leftSubtree.begin(), leftSubtree.end());

    std::vector<int> rightSubtree = postOrderTreeTraversal(root->getRightChild());
    result.insert(result.end(), rightSubtree.begin(), rightSubtree.end());

    result.push_back(root->getPrice());

    return result;
}

// Return a random active ticket
// 0:Limit, 1:Stop, 2:StopLimit
Ticket* Ledger::getRandomTicket(int key, std::mt19937 gen) const
{
    if (key == 0)
    {
        if (limitTickets.size() > 10000)
        {
            // Generate a random index within the range of the hash set size
            std::uniform_int_distribution<> mapDist(0, limitTickets.size() - 1);
            int randomIndex = mapDist(gen);

            // Access the element at the random index directly
            auto it = limitTickets.begin();
            std::advance(it, randomIndex);
            return *it;
        }
        return nullptr;
    } else if (key == 1)
    {
        if (stopTickets.size() > 500)
        {
            // Generate a random index within the range of the hash set size
            std::uniform_int_distribution<> mapDist(0, stopTickets.size() - 1);
            int randomIndex = mapDist(gen);

            // Access the element at the random index directly
            auto it = stopTickets.begin();
            std::advance(it, randomIndex);
            return *it;
        }
        return nullptr;
    } else if (key == 2)
    {
        if (stopLimitTickets.size() > 500)
        {
            // Generate a random index within the range of the hash set size
            std::uniform_int_distribution<> mapDist(0, stopLimitTickets.size() - 1);
            int randomIndex = mapDist(gen);

            // Access the element at the random index directly
            auto it = stopLimitTickets.begin();
            std::advance(it, randomIndex);
            return *it;
        }
        return nullptr;
    }
    return nullptr;
}

// Add a new price level to the ledger
void Ledger::addPriceLevel(int price, bool isBuySide)
{
    auto& levelMap = isBuySide ? bidLevels : askLevels;
    auto& tree = isBuySide ? bidTree : askTree;
    auto& ledgerEdge = isBuySide ? bestBid : bestAsk;

    PriceLevel* newLevel = new PriceLevel(price, isBuySide);
    levelMap.emplace(price, newLevel);

    if (tree == nullptr)
    {
        tree = newLevel;
        ledgerEdge = newLevel;
    } else
    {
        PriceLevel* root = insert(tree, newLevel);
        updateLedgerEdgeInsert(newLevel);
    }
}

// Add a new stop level to the ledger
void Ledger::addStopLevel(int stopPrice, bool isBuySide)
{
    auto& tree = isBuySide ? stopBidTree : stopAskTree;
    auto& ledgerEdge = isBuySide ? lowestStopBid : highestStopAsk;

    PriceLevel* newStop = new PriceLevel(stopPrice, isBuySide);
    stopLevels.emplace(stopPrice, newStop);

    if (tree == nullptr)
    {
        tree = newStop;
        ledgerEdge = newStop;
    } else
    {
        PriceLevel* root = insertStop(tree, newStop);
        updateStopLedgerEdgeInsert(newStop);
    }
}

// Insert a price level into its binary search tree
PriceLevel* Ledger::insert(PriceLevel* root, PriceLevel* level, PriceLevel* parent)
{
    if (root == nullptr)
    {
        level->setParent(parent);
        return level;
    }
    if (level->getPrice() < root->getPrice())
    {
        root->setLeftChild(insert(root->getLeftChild(), level, root));
        root = balance(root);
    } else if (level->getPrice() > root->getPrice())
    {
        root->setRightChild(insert(root->getRightChild(), level, root));
        root = balance(root);
    }

    return root;
}

// Insert a price level into its stop binary search tree
PriceLevel* Ledger::insertStop(PriceLevel* root, PriceLevel* level, PriceLevel* parent)
{
    if (root == nullptr)
    {
        level->setParent(parent);
        return level;
    }
    if (level->getPrice() < root->getPrice())
    {
        root->setLeftChild(insertStop(root->getLeftChild(), level, root));
        root = balanceStop(root);
    } else if (level->getPrice() > root->getPrice())
    {
        root->setRightChild(insertStop(root->getRightChild(), level, root));
        root = balanceStop(root);
    }
    return root;
}

// Update the edge of the ledger if new level is on edge of the ledger
void Ledger::updateLedgerEdgeInsert(PriceLevel* newLevel)
{
    if (newLevel->getIsBuySide())
    {
        if (newLevel->getPrice() > bestBid->getPrice())
        {
            bestBid = newLevel;
        }
    } else
    {
        if (newLevel->getPrice() < bestAsk->getPrice())
        {
            bestAsk = newLevel;
        }
    }
}

// Update the edge of the stop ledger if new stop is on edge of the ledger
void Ledger::updateStopLedgerEdgeInsert(PriceLevel* newStop)
{
    if (newStop->getIsBuySide())
    {
        if (newStop->getPrice() < lowestStopBid->getPrice())
        {
            lowestStopBid = newStop;
        }
    } else
    {
        if (newStop->getPrice() > highestStopAsk->getPrice())
        {
            highestStopAsk = newStop;
        }
    }
}

// Update the edge of the ledger if current edge of the ledger is emptied
void Ledger::updateLedgerEdgeRemove(PriceLevel* level)
{
    auto& ledgerEdge = level->getIsBuySide() ? bestBid : bestAsk;
    auto& tree = level->getIsBuySide() ? bidTree : askTree;

    if (level == ledgerEdge)
    {
        if (ledgerEdge != tree)
        {
            if (level->getIsBuySide() && ledgerEdge->getLeftChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getLeftChild();
            } else if (!level->getIsBuySide() && ledgerEdge->getRightChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getRightChild();
            } else {
            ledgerEdge = ledgerEdge->getParent();
            }
        } else {
            if (level->getIsBuySide() && ledgerEdge->getLeftChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getLeftChild();
            } else if (!level->getIsBuySide() && ledgerEdge->getRightChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getRightChild();
            } else {
            ledgerEdge = nullptr;
            }
        }
    }
}

// Update the edge of the stop ledger if current edge of the stop ledger is emptied
void Ledger::updateStopLedgerEdgeRemove(PriceLevel* stopLevel)
{
    auto& ledgerEdge = stopLevel->getIsBuySide() ? lowestStopBid : highestStopAsk;
    auto& tree = stopLevel->getIsBuySide() ? stopBidTree : stopAskTree;

    if (stopLevel == ledgerEdge)
    {
        if (ledgerEdge != tree)
        {
            if (stopLevel->getIsBuySide() && ledgerEdge->getRightChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getRightChild();
            } else if (!stopLevel->getIsBuySide() && ledgerEdge->getLeftChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getLeftChild();
            } else {
            ledgerEdge = ledgerEdge->getParent();
            }
        } else {
            if (stopLevel->getIsBuySide() && ledgerEdge->getRightChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getRightChild();
            } else if (!stopLevel->getIsBuySide() && ledgerEdge->getLeftChild() != nullptr)
            {
                ledgerEdge = ledgerEdge->getLeftChild();
            } else {
                ledgerEdge = nullptr;
            }
        }
    }
}

// Change the root price level in the AVL tree if the root level is deleted
void Ledger::changeLedgerRoots(PriceLevel* level){
    auto& tree = level->getIsBuySide() ? bidTree : askTree;
    if (level == tree)
    {
        if (level->getRightChild() != nullptr)
        {
            tree = tree->getRightChild();
            while (tree->getLeftChild() != nullptr)
            {
                tree = tree->getLeftChild();
            }
        } else
        {
            tree = level->getLeftChild();
        }
    }
}

// Change the root stop level in the AVL tree if the root stop level is deleted
void Ledger::changeStopLedgerRoots(PriceLevel* stopLevel){
    auto& tree = stopLevel->getIsBuySide() ? stopBidTree : stopAskTree;
    if (stopLevel == tree)
    {
        if (stopLevel->getRightChild() != nullptr)
        {
            tree = tree->getRightChild();
            while (tree->getLeftChild() != nullptr)
            {
                tree = tree->getLeftChild();
            }
        } else
        {
            tree = stopLevel->getLeftChild();
        }
    }
}

// Delete a price level after it has been emptied
void Ledger::deletePriceLevel(PriceLevel* level)
{
    updateLedgerEdgeRemove(level);
    deleteFromLevelMaps(level->getPrice(), level->getIsBuySide());
    changeLedgerRoots(level);

    PriceLevel* parent = level->getParent();
    int price = level->getPrice();
    delete level;
    while (parent != nullptr)
    {
        parent = balance(parent);
        if (parent->getParent() != nullptr)
        {
            if (parent->getParent()->getPrice() > price)
            {
                parent->getParent()->setLeftChild(parent);
            } else {
                parent->getParent()->setRightChild(parent);
            }
        }
        parent = parent->getParent();
    }
}

// Delete a stop level after it has been emptied
void Ledger::deleteStopLevel(PriceLevel* stopLevel)
{
    updateStopLedgerEdgeRemove(stopLevel);
    deleteFromStopLevels(stopLevel->getPrice());
    changeStopLedgerRoots(stopLevel);

    PriceLevel* parent = stopLevel->getParent();
    int stopPrice = stopLevel->getPrice();
    delete stopLevel;
    while (parent != nullptr)
    {
        parent = balanceStop(parent);
        if (parent->getParent() != nullptr)
        {
            if (parent->getParent()->getPrice() > stopPrice)
            {
                parent->getParent()->setLeftChild(parent);
            } else {
                parent->getParent()->setRightChild(parent);
            }
        }
        parent = parent->getParent();
    }
}

// Delete a ticket from the ticket registry
void Ledger::deleteFromTicketRegistry(int ticketId)
{
    ticketRegistry.erase(ticketId);
}

// Delete a price level from the level maps
void Ledger::deleteFromLevelMaps(int price, bool isBuySide)
{
    auto& levelMap = isBuySide ? bidLevels : askLevels;
    levelMap.erase(price);
}

// Delete a stop level from the stop levels map
void Ledger::deleteFromStopLevels(int stopPrice)
{
    stopLevels.erase(stopPrice);
}

// When a limit ticket overlaps with the best bid or best ask, immediately
// execute it as if it were a market ticket
int Ledger::limitTicketAsMarketTicket(int ticketId, bool isBuySide, int quantity, int price)
{
    if (isBuySide)
    {
        while (bestAsk != nullptr && quantity != 0 && bestAsk->getPrice() <= price)
        {
            if (quantity <= bestAsk->getAggregateQty())
            {
                marketTicketHelper(ticketId, isBuySide, quantity);
                return 0;
            } else {
                quantity -= bestAsk->getAggregateQty();
                marketTicketHelper(ticketId, isBuySide, bestAsk->getAggregateQty());
            }
        }
        return quantity;
    } else {
        while (bestBid != nullptr && quantity != 0 && bestBid->getPrice() >= price)
        {
            if (quantity <= bestBid->getAggregateQty())
            {
                marketTicketHelper(ticketId, isBuySide, quantity);
                return 0;
            } else {
                quantity -= bestBid->getAggregateQty();
                marketTicketHelper(ticketId, isBuySide, bestBid->getAggregateQty());
            }
        }
        return quantity;
    }
}

// When a stop ticket overlaps with the best bid or best ask, immediately
// execute it as if it were a market ticket
int Ledger::stopTicketAsMarketTicket(int ticketId, bool isBuySide, int quantity, int stopPrice)
{
    if (isBuySide && bestAsk != nullptr && stopPrice <= bestAsk->getPrice())
    {
        marketTicket(ticketId, true, quantity);
        return 0;
    } else if (!isBuySide && bestBid != nullptr && stopPrice >= bestBid->getPrice())
    {
        marketTicket(ticketId, false, quantity);
        return 0;
    }
    return quantity;
}

// When a limit ticket that used to be a stop limit ticket overlaps with the best bid or best ask,
// immediately execute it as if it were a market ticket
int Ledger::existingTicketAsMarketTicket(Ticket* frontTicket, bool isBuySide)
{
    int quantity = frontTicket->getQuantity();
    int ticketId = frontTicket->getTicketId();
    int price = frontTicket->getPriceRef();

    if (isBuySide)
    {
        while (bestAsk != nullptr && bestAsk->getPrice() <= price)
        {
            if (quantity <= bestAsk->getAggregateQty())
            {
                deleteFromTicketRegistry(ticketId);
                delete frontTicket;
                marketTicketHelper(ticketId, isBuySide, quantity);
                return 0;
            } else {
                quantity -= bestAsk->getAggregateQty();
                marketTicketHelper(ticketId, isBuySide, bestAsk->getAggregateQty());
            }
        }
        return quantity;
    } else {
        while (bestBid != nullptr && bestBid->getPrice() >= price)
        {
            if (quantity <= bestBid->getAggregateQty())
            {
                deleteFromTicketRegistry(ticketId);
                delete frontTicket;
                marketTicketHelper(ticketId, isBuySide, quantity);
                return 0;
            } else {
                quantity -= bestBid->getAggregateQty();
                marketTicketHelper(ticketId, isBuySide, bestBid->getAggregateQty());
            }
        }
        return quantity;
    }
}

// When a stop limit ticket overlaps with the best bid or best ask, immediately
// execute it as if it were a limit ticket
int Ledger::stopLimitTicketAsLimitTicket(int ticketId, bool isBuySide, int quantity, int price, int stopPrice)
{
    if (isBuySide && bestAsk != nullptr && stopPrice <= bestAsk->getPrice())
    {
        addLimitTicket(ticketId, true, quantity, price);
        return 0;
    } else if (!isBuySide && bestBid != nullptr && stopPrice >= bestBid->getPrice())
    {
        addLimitTicket(ticketId, false, quantity, price);
        return 0;
    }
    return quantity;
}

// Executes any stop tickets which need to be executed
void Ledger::executeStopTickets(bool isBuySide)
{
    if (isBuySide)
    {
        // Execute any buy stop market tickets
        // If the ledger is empty and can't complete stop market ticket then it just doesn't execute and is forgotten.
        while (lowestStopBid != nullptr && (bestAsk == nullptr || lowestStopBid->getPrice() <= bestAsk->getPrice()))
        {
            Ticket* frontTicket = lowestStopBid->getFrontTicket();
            if (frontTicket->getPriceRef() == 0)
            {
                int quantity = frontTicket->getQuantity();
                frontTicket->execute();
                if (lowestStopBid->getTicketCount() == 0)
                {
                    deleteStopLevel(lowestStopBid);
                }
                deleteFromTicketRegistry(frontTicket->getTicketId());
                // stopTickets.erase(frontTicket);
                delete frontTicket;
                marketTicketHelper(0, true, quantity);
            } else {
                // stopLimitTickets.erase(frontTicket);
                stopLimitTicketToLimitTicket(frontTicket, isBuySide);
            }
        }
    } else {
        // Execute any sell stop market tickets
        // If the ledger is empty and can't complete stop market ticket then it just doesn't execute and is forgotten.
        while (highestStopAsk != nullptr && (bestBid == nullptr || highestStopAsk->getPrice() >= bestBid->getPrice()))
        {
            Ticket* frontTicket = highestStopAsk->getFrontTicket();
            if (frontTicket->getPriceRef() == 0)
            {
                int quantity = frontTicket->getQuantity();
                frontTicket->execute();
                if (highestStopAsk->getTicketCount() == 0)
                {
                    deleteStopLevel(highestStopAsk);
                }
                deleteFromTicketRegistry(frontTicket->getTicketId());
                // stopTickets.erase(frontTicket);
                delete frontTicket;
                marketTicketHelper(0, false, quantity);
            } else {
                // stopLimitTickets.erase(frontTicket);
                stopLimitTicketToLimitTicket(frontTicket, isBuySide);
            }
        }
    }
}

// Turn stop limit ticket into limit ticket
void Ledger::stopLimitTicketToLimitTicket(Ticket* frontTicket, bool isBuySide)
{
    auto& ledgerEdge = isBuySide ? lowestStopBid : highestStopAsk;
    frontTicket->execute();
    if (ledgerEdge->getTicketCount() == 0)
    {
        deleteStopLevel(ledgerEdge);
    }

    // Account for ticket being executed immediately - majority of cases
    int quantity = existingTicketAsMarketTicket(frontTicket, isBuySide);

    if (quantity != 0)
    {
        frontTicket->setQuantity(quantity);
        auto& levelMap = isBuySide ? bidLevels : askLevels;

        if (levelMap.find(frontTicket->getPriceRef()) == levelMap.end())
        {
            addPriceLevel(frontTicket->getPriceRef(), isBuySide);
        }
        levelMap.at(frontTicket->getPriceRef())->append(frontTicket);
        // limitTickets.insert(frontTicket);
    }
}

// Function which actually executes the market ticket.
// If the ledger is empty and can't complete market ticket then market ticket just doesn't execute and is forgotten
void Ledger::marketTicketHelper(int ticketId, bool isBuySide, int quantity)
{
    auto& ledgerEdge = isBuySide ? bestAsk : bestBid;

    while (ledgerEdge != nullptr && ledgerEdge->getFrontTicket()->getQuantity() <= quantity)
    {
        Ticket* frontTicket = ledgerEdge->getFrontTicket();
        quantity -= frontTicket->getQuantity();
        frontTicket->execute();
        if (ledgerEdge->getTicketCount() == 0)
        {
            deletePriceLevel(ledgerEdge);
        }
        deleteFromTicketRegistry(frontTicket->getTicketId());
        // limitTickets.erase(frontTicket);
        delete frontTicket;
        fillCount += 1;
    }
    if (ledgerEdge != nullptr && quantity != 0)
    {
        ledgerEdge->getFrontTicket()->partiallyFillTicket(quantity);
        fillCount += 1;
    }
}

// Get height difference between a price level's children
int Ledger::levelHeightDifference(PriceLevel* level) {
    int l_height = getLevelHeight(level->getLeftChild());
    int r_height = getLevelHeight(level->getRightChild());
    int b_factor = l_height - r_height;
    return b_factor;
}

// RR rotation for AVL restructure
PriceLevel* Ledger::rr_rotate(PriceLevel* parent) {
    PriceLevel* newParent = parent->getRightChild();
    parent->setRightChild(newParent->getLeftChild());
    if (newParent->getLeftChild() != nullptr)
    {
        newParent->getLeftChild()->setParent(parent);
    }
    newParent->setLeftChild(parent);
    if (parent->getParent() != nullptr)
    {
        newParent->setParent(parent->getParent());
    } else {
        newParent->setParent(nullptr);
        auto& tree = parent->getIsBuySide() ? bidTree : askTree;
        tree = newParent;
    }
    parent->setParent(newParent);
    return newParent;
}

// LL rotation for AVL restructure
PriceLevel* Ledger::ll_rotate(PriceLevel* parent) {
    PriceLevel* newParent = parent->getLeftChild();
    parent->setLeftChild(newParent->getRightChild());
    if (newParent->getRightChild() != nullptr)
    {
        newParent->getRightChild()->setParent(parent);
    }
    newParent->setRightChild(parent);
    if (parent->getParent() != nullptr)
    {
        newParent->setParent(parent->getParent());
    } else {
        newParent->setParent(nullptr);
        auto& tree = parent->getIsBuySide() ? bidTree : askTree;
        tree = newParent;
    }
    parent->setParent(newParent);
    return newParent;
}

// LR rotation for AVL restructure
PriceLevel* Ledger::lr_rotate(PriceLevel* parent) {
    PriceLevel* newParent = parent->getLeftChild();
    parent->setLeftChild(rr_rotate(newParent));
    return ll_rotate(parent);
}

// RL rotation for AVL restructure
PriceLevel* Ledger::rl_rotate(PriceLevel* parent) {
    PriceLevel* newParent = parent->getRightChild();
    parent->setRightChild(ll_rotate(newParent));
    return rr_rotate(parent);
}

// Check if the AVL tree needs to be restructured
PriceLevel* Ledger::balance(PriceLevel* level) {
    int bal_factor = levelHeightDifference(level);
    if (bal_factor > 1) {
        if (levelHeightDifference(level->getLeftChild()) >= 0)
            level = ll_rotate(level);
        else
            level = lr_rotate(level);
        rebalanceCount += 1;
    } else if (bal_factor < -1) {
        if (levelHeightDifference(level->getRightChild()) > 0)
            level = rl_rotate(level);
        else
            level = rr_rotate(level);
        rebalanceCount += 1;
    }
    return level;
}

// RR rotation for AVL stop tree restructure
PriceLevel* Ledger::rr_rotateStop(PriceLevel* parent) {
    PriceLevel* newParent = parent->getRightChild();
    parent->setRightChild(newParent->getLeftChild());
    if (newParent->getLeftChild() != nullptr)
    {
        newParent->getLeftChild()->setParent(parent);
    }
    newParent->setLeftChild(parent);
    if (parent->getParent() != nullptr)
    {
        newParent->setParent(parent->getParent());
    } else {
        newParent->setParent(nullptr);
        auto& tree = parent->getIsBuySide() ? stopBidTree : stopAskTree;
        tree = newParent;
    }
    parent->setParent(newParent);
    return newParent;
}

// LL rotation for AVL stop tree restructure
PriceLevel* Ledger::ll_rotateStop(PriceLevel* parent) {
    PriceLevel* newParent = parent->getLeftChild();
    parent->setLeftChild(newParent->getRightChild());
    if (newParent->getRightChild() != nullptr)
    {
        newParent->getRightChild()->setParent(parent);
    }
    newParent->setRightChild(parent);
    if (parent->getParent() != nullptr)
    {
        newParent->setParent(parent->getParent());
    } else {
        newParent->setParent(nullptr);
        auto& tree = parent->getIsBuySide() ? stopBidTree : stopAskTree;
        tree = newParent;
    }
    parent->setParent(newParent);
    return newParent;
}

// LR rotation for AVL stop tree restructure
PriceLevel* Ledger::lr_rotateStop(PriceLevel* parent) {
    PriceLevel* newParent = parent->getLeftChild();
    parent->setLeftChild(rr_rotateStop(newParent));
    return ll_rotateStop(parent);
}

// RL rotation for AVL stop tree restructure
PriceLevel* Ledger::rl_rotateStop(PriceLevel* parent) {
    PriceLevel* newParent = parent->getRightChild();
    parent->setRightChild(ll_rotateStop(newParent));
    return rr_rotateStop(parent);
}

// Check if the AVL stop tree needs to be restructured
PriceLevel* Ledger::balanceStop(PriceLevel* level) {
    int bal_factor = levelHeightDifference(level);
    if (bal_factor > 1) {
        if (levelHeightDifference(level->getLeftChild()) >= 0)
            level = ll_rotateStop(level);
        else
            level = lr_rotateStop(level);
        rebalanceCount += 1;
    } else if (bal_factor < -1) {
        if (levelHeightDifference(level->getRightChild()) > 0)
            level = rl_rotateStop(level);
        else
            level = rr_rotateStop(level);
        rebalanceCount += 1;
    }
    return level;
}
