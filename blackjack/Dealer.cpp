#include "Dealer.h"

Dealer::Dealer()
    : hand()
{
}

void Dealer::clearHand()
{
    hand.clear();
}

void Dealer::addCard(const Card& card)
{
    hand.addCard(card);
}

void Dealer::playTurn(Deck& deck)
{
    while (hand.getValue() < 17)
    {
        hand.addCard(deck.drawCard());
    }
}

const Hand& Dealer::getHand() const
{
    return hand;
}

int Dealer::getValue() const
{
    return hand.getValue();
}

std::string Dealer::toString(bool hideHoleCard) const
{
    if (!hideHoleCard)
    {
        return hand.toString();
    }

    const auto& cards = hand.getCards();
    std::string result;

    for (size_t i = 0; i < cards.size(); ++i)
    {
        if (i == 0)
            result += "[" + cards[i].toShortString() + "]";
        else
            result += "[?]";

        if (i + 1 < cards.size())
            result += " ";
    }

    return result;
}

int Dealer::getUpCardValue() const
{
    const auto& cards = hand.getCards();
    if (cards.empty())
        return 0;

    return cards[0].getValue();
}

bool Dealer::hasSoft17() const
{
    int value = hand.getValue();
    bool hasAce = false;

    for (const auto& c : hand.getCards())
        if (c.getRank() == "A")
            hasAce = true;

    return (value == 17 && hasAce);
}
