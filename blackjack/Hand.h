#ifndef HAND_H
#define HAND_H

#include "Card.h"
#include <vector>
#include <string>

class Hand
{
public:
    Hand();
    void addCard(const Card& card);
    void clear();
    int getValue() const;
    bool isBlackjack() const;
    bool isBust() const;
    std::string toString() const;
    const std::vector<Card>& getCards() const;
private:
    std::vector<Card> cards;
};

#endif
