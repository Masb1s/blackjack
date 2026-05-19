#include "Card.h"
#include <sstream>

Card::Card(const std::string& rank, Suit suit)
    : rank(rank), suit(suit)
{
}

std::string Card::getRank() const
{
    return rank;
}

Suit Card::getSuit() const
{
    return suit;
}

int Card::getBaseValue() const
{
    if (rank == "A")
    {
        return 11;
    }
    if (rank == "K" || rank == "Q" || rank == "J")
    {
        return 10;
    }
    try
    {
        int v = std::stoi(rank);
        return v;
    }
    catch (...)
    {
        return 0;
    }
}

std::string Card::toShortString() const
{
    char suitChar[4] = { 0 };
    switch (suit)
    {
    case Suit::Clubs:
        suitChar[0] = '\xE2';
        suitChar[1] = '\x99';
        suitChar[2] = '\xA3';
        break;
    case Suit::Diamonds:
        suitChar[0] = '\xE2';
        suitChar[1] = '\x99';
        suitChar[2] = '\xA6';
        break;
    case Suit::Hearts:
        suitChar[0] = '\xE2';
        suitChar[1] = '\x99';
        suitChar[2] = '\xA5';
        break;
    case Suit::Spades:
        suitChar[0] = '\xE2';
        suitChar[1] = '\x99';
        suitChar[2] = '\xA0';
        break;
    }
    std::ostringstream oss;
    oss << rank << suitChar;
    return oss.str();
}
