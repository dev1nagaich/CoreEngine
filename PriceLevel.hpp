#ifndef PRICELEVEL_HPP
#define PRICELEVEL_HPP

class Ticket;

class PriceLevel {
private:
    int price;
    int ticketCount;
    int aggregateQty;
    bool isBuySide;
    PriceLevel *parent;
    PriceLevel *leftChild;
    PriceLevel *rightChild;
    Ticket *frontTicket;
    Ticket *backTicket;

    friend class Ticket;
public:
    PriceLevel(int _price, bool _isBuySide, int _ticketCount=0, int _aggregateQty=0);
    ~PriceLevel();

    Ticket* getFrontTicket() const;
    int getPrice() const;
    int getTicketCount() const;
    int getAggregateQty() const;
    bool getIsBuySide() const;
    PriceLevel* getParent() const;
    PriceLevel* getLeftChild() const;
    PriceLevel* getRightChild() const;
    void setParent(PriceLevel* newParent);
    void setLeftChild(PriceLevel* newLeftChild);
    void setRightChild(PriceLevel* newRightChild);
    void partiallyFillAggregateQty(int filledQty);

    void append(Ticket *_ticket);

    void printForward() const;
    void printBackward() const;
    void print() const;
};

#endif
