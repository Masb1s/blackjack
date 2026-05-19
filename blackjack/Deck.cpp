#include "Deck.h"
#include <algorithm>
#include <chrono>

Deck::Deck()
{
    std::random_device rd;
    rng = std::mt19937(rd());
    initialize();
    shuffleDeck();
}

std::vector<std::string> Deck::getRanks() const
{
    std::vector<std::string> ranks;
    for (int i = 2; i <= 10; ++i)
    {
        ranks.push_back(std::to_string(i));
    }
    std::vector<std::string> face = { "J", "Q", "K", "A" };
    for (const auto& r : face)
    {
        ranks.push_back(r);
    }
    return ranks;
}

void Deck::initialize()
{
    cards.clear();
    std::vector<std::string> ranks = getRanks();
    for (int s = 0; s < 4; ++s)
    {
        for (const auto& r : ranks)
        {
            cards.emplace_back(r, static_cast<Suit>(s));
        }
    }
}

void Deck::shuffleDeck()
{
    std::shuffle(cards.begin(), cards.end(), rng);
}

Card Deck::drawCard()
{
    if (cards.empty())
    {
        initialize();
        shuffleDeck();
    }
    Card c = cards.back();
    cards.pop_back();
    return c;
}

bool Deck::isEmpty() const
{
    return cards.empty();
}

int Deck::size() const
{
    return static_cast<int>(cards.size());
}
