#ifndef DEALER_H
#define DEALER_H

#include "Hand.h"
#include "Deck.h"
#include <string>

class Dealer
{
public:
    Dealer();
    void clearHand();
    void addCard(const Card& card);
    void playTurn(Deck& deck);
    const Hand& getHand() const;
    int getValue() const;
    std::string toString(bool hideHoleCard) const;
private:
    Hand hand;
};

#endif
