#ifndef TICKET_HPP
#define TICKET_HPP

class PriceLevel;

class Ticket {
private:
    int ticketId;
    bool isBuySide;
    int quantity;
    int priceRef;
    Ticket *nextTicket;
    Ticket *prevTicket;
    PriceLevel *ownerLevel;

    friend class PriceLevel;
public:
    Ticket(int _ticketId, bool _isBuySide, int _quantity, int _priceRef);

    int getQuantity() const;
    int getTicketId() const;
    bool getIsBuySide() const;
    int getPriceRef() const;
    PriceLevel* getOwnerLevel() const;

    void partiallyFillTicket(int filledQty);
    void cancel();
    void execute();
    void modifyTicket(int newQuantity, int newPriceRef);
    void setQuantity(int newQuantity);

    void print() const;
};

#endif
