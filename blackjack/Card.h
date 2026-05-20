#ifndef CARD_H
#define CARD_H

#include <string>

enum class Suit
{
    Clubs,
    Diamonds,
    Hearts,
    Spades
};

class Card
{
public:
    Card(const std::string& rank, Suit suit);

    std::string getRank() const;
    Suit getSuit() const;

    int getBaseValue() const;
    int getValue() const; 

    std::string toShortString() const;

private:
    std::string rank;
    Suit suit;
};

#endif
