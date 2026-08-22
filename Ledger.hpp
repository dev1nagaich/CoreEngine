#ifndef LEDGER_HPP
#define LEDGER_HPP

#include <unordered_map>
#include <vector>
#include <random>
#include <unordered_set>

class PriceLevel;
class Ticket;

class Ledger {
private:
    PriceLevel *bidTree;
    PriceLevel *askTree;
    PriceLevel *bestAsk;
    PriceLevel *bestBid;

    PriceLevel *stopBidTree;
    PriceLevel *stopAskTree;
    PriceLevel *highestStopAsk;
    PriceLevel *lowestStopBid;

    std::unordered_map<int, Ticket*> ticketRegistry;
    std::unordered_map<int, PriceLevel*> bidLevels;
    std::unordered_map<int, PriceLevel*> askLevels;
    std::unordered_map<int, PriceLevel*> stopLevels;

    void addPriceLevel(int price, bool isBuySide);
    void addStopLevel(int stopPrice, bool isBuySide);
    PriceLevel* insert(PriceLevel* root, PriceLevel* level, PriceLevel* parent=nullptr);
    PriceLevel* insertStop(PriceLevel* root, PriceLevel* level, PriceLevel* parent=nullptr);
    void updateLedgerEdgeInsert(PriceLevel* newLevel);
    void updateStopLedgerEdgeInsert(PriceLevel* newStop);
    void updateLedgerEdgeRemove(PriceLevel* level);
    void updateStopLedgerEdgeRemove(PriceLevel* stopLevel);
    void changeLedgerRoots(PriceLevel* level);
    void changeStopLedgerRoots(PriceLevel* stopLevel);
    void deletePriceLevel(PriceLevel* level);
    void deleteStopLevel(PriceLevel* level);
    void deleteFromTicketRegistry(int ticketId);
    void deleteFromLevelMaps(int price, bool isBuySide);
    void deleteFromStopLevels(int stopPrice);
    int limitTicketAsMarketTicket(int ticketId, bool isBuySide, int quantity, int price);
    int stopTicketAsMarketTicket(int ticketId, bool isBuySide, int quantity, int stopPrice);
    int existingTicketAsMarketTicket(Ticket* frontTicket, bool isBuySide);
    int stopLimitTicketAsLimitTicket(int ticketId, bool isBuySide, int quantity, int price, int stopPrice);
    void executeStopTickets(bool isBuySide);
    void stopLimitTicketToLimitTicket(Ticket* frontTicket, bool isBuySide);
    void marketTicketHelper(int ticketId, bool isBuySide, int quantity);

    // Functions to balance AVL tree
    int levelHeightDifference(PriceLevel* level);
    PriceLevel* rr_rotate(PriceLevel* level);
    PriceLevel* ll_rotate(PriceLevel* level);
    PriceLevel* lr_rotate(PriceLevel* level);
    PriceLevel* rl_rotate(PriceLevel* level);
    PriceLevel* balance(PriceLevel* level);
    PriceLevel* rr_rotateStop(PriceLevel* level);
    PriceLevel* ll_rotateStop(PriceLevel* level);
    PriceLevel* lr_rotateStop(PriceLevel* level);
    PriceLevel* rl_rotateStop(PriceLevel* level);
    PriceLevel* balanceStop(PriceLevel* level);

public:
    Ledger();
    ~Ledger();

    // Counts used in order book performance visualisations
    int fillCount=0;
    int rebalanceCount=0;

    // Getter and setter functions
    PriceLevel* getBidTree() const;
    PriceLevel* getAskTree() const;
    PriceLevel* getBestAsk() const;
    PriceLevel* getBestBid() const;
    PriceLevel* getStopBidTree() const;
    PriceLevel* getStopAskTree() const;
    PriceLevel* getHighestStopAsk() const;
    PriceLevel* getLowestStopBid() const;

    // Functions for different types of tickets
    void marketTicket(int ticketId, bool isBuySide, int quantity);
    void addLimitTicket(int ticketId, bool isBuySide, int quantity, int price);
    void cancelLimitTicket(int ticketId);
    void modifyLimitTicket(int ticketId, int newQuantity, int newPrice);
    void addStopTicket(int ticketId, bool isBuySide, int quantity, int stopPrice);
    void cancelStopTicket(int ticketId);
    void modifyStopTicket(int ticketId, int newQuantity, int newStopPrice);
    void addStopLimitTicket(int ticketId, bool isBuySide, int quantity, int price, int stopPrice);
    void cancelStopLimitTicket(int ticketId);
    void modifyStopLimitTicket(int ticketId, int newQuantity, int newPrice, int newStopPrice);

    // Functions that needed to be public for testing purposes
    int getLevelHeight(PriceLevel* level) const;
    Ticket* searchTicketRegistry(int ticketId) const;
    PriceLevel* searchLevelMaps(int price, bool isBuySide) const;
    PriceLevel* searchStopLevels(int stopPrice) const;

    // Functions for visualising the order book
    void printPriceLevel(int price, bool isBuySide) const;
    void printTicket(int ticketId) const;
    void printLedgerEdges() const;
    void printLedger() const;
    std::vector<int> inOrderTreeTraversal(PriceLevel* root) const;
    std::vector<int> preOrderTreeTraversal(PriceLevel* root) const;
    std::vector<int> postOrderTreeTraversal(PriceLevel* root) const;

    // Functions and data structures needed for generating sample data
    Ticket* getRandomTicket(int key, std::mt19937 gen) const;
    std::unordered_set<Ticket*> limitTickets;
    std::unordered_set<Ticket*> stopTickets;
    std::unordered_set<Ticket*> stopLimitTickets;
};

#endif
