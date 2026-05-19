#include "Hand.h"
#include <sstream>

Hand::Hand()
    : cards()
{
}

void Hand::addCard(const Card& card)
{
    cards.push_back(card);
}

void Hand::clear()
{
    cards.clear();
}

int Hand::getValue() const
{
    int total = 0;
    int aces = 0;
    for (const auto& c : cards)
    {
        std::string r = c.getRank();
        if (r == "A")
        {
            ++aces;
            total += 1;
        }
        else if (r == "K" || r == "Q" || r == "J")
        {
            total += 10;
        }
        else
        {
            total += std::stoi(r);
        }
    }
    for (int i = 0; i < aces; ++i)
    {
        if (total + 10 <= 21)
        {
            total += 10;
        }
    }
    return total;
}

bool Hand::isBlackjack() const
{
    return cards.size() == 2 && getValue() == 21;
}

bool Hand::isBust() const
{
    return getValue() > 21;
}

std::string Hand::toString() const
{
    std::ostringstream oss;
    for (size_t i = 0; i < cards.size(); ++i)
    {
        oss << "[" << cards[i].toShortString() << "]";
        if (i + 1 < cards.size())
        {
            oss << " ";
        }
    }
    oss << " = " << getValue();
    return oss.str();
}

const std::vector<Card>& Hand::getCards() const
{
    return cards;
}
