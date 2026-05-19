#include "Player.h"

Player::Player(int initialBalance)
    : balance(initialBalance), currentBet(0), hands(), bets()
{
}

int Player::getBalance() const
{
    return balance;
}

int Player::getCurrentBet() const
{
    return currentBet;
}

void Player::setCurrentBet(int amount)
{
    currentBet = amount;
}

void Player::increaseBalance(int amount)
{
    balance += amount;
}

void Player::decreaseBalance(int amount)
{
    balance -= amount;
    if (balance < 0)
    {
        balance = 0;
    }
}

bool Player::placeBet(int amount)
{
    if (amount <= 0 || amount > balance)
    {
        return false;
    }
    currentBet = amount;
    decreaseBalance(amount);
    hands.clear();
    bets.clear();
    hands.emplace_back();
    bets.push_back(amount);
    return true;
}

void Player::clearHands()
{
    hands.clear();
    bets.clear();
}

void Player::addHand(const Hand& hand)
{
    hands.push_back(hand);
}

std::vector<Hand>& Player::getHands()
{
    return hands;
}

const std::vector<Hand>& Player::getHands() const
{
    return hands;
}

bool Player::canSplit() const
{
    if (hands.empty())
    {
        return false;
    }
    const Hand& first = hands.front();
    const auto& cards = first.getCards();
    if (cards.size() != 2)
    {
        return false;
    }
    return cards[0].getRank() == cards[1].getRank();
}

bool Player::performSplit()
{
    if (!canSplit())
    {
        return false;
    }
    if (bets.empty())
    {
        return false;
    }
    int originalBet = bets[0];
    if (originalBet > balance)
    {
        return false;
    }
    Hand firstHand = hands[0];
    const auto& cards = firstHand.getCards();
    Hand handA;
    Hand handB;
    if (cards.size() >= 1)
    {
        handA.addCard(cards[0]);
    }
    if (cards.size() >= 2)
    {
        handB.addCard(cards[1]);
    }
    hands.clear();
    hands.push_back(handA);
    hands.push_back(handB);
    bets.clear();
    bets.push_back(originalBet);
    bets.push_back(originalBet);
    decreaseBalance(originalBet);
    return true;
}

std::vector<int> Player::getBets() const
{
    return bets;
}

void Player::setBetForHand(size_t index, int amount)
{
    if (index < bets.size() && amount >= 0)
    {
        bets[index] = amount;
    }
}