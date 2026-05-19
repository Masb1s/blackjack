#ifndef DECK_H
#define DECK_H

#include "Card.h"
#include <vector>
#include <random>
#include <string>

class Deck
{
public:
    Deck();
    void initialize();
    void shuffleDeck();
    Card drawCard();
    bool isEmpty() const;
    int size() const;
private:
    std::vector<Card> cards;
    std::mt19937 rng;
    std::vector<std::string> getRanks() const;
};

#endif
