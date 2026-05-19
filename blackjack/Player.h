#ifndef PLAYER_H
#define PLAYER_H

#include "Hand.h"
#include <vector>
#include <string>

class Player
{
public:
    Player(int initialBalance = 1000);
    int getBalance() const;
    int getCurrentBet() const;
    void setCurrentBet(int amount);
    void increaseBalance(int amount);
    void decreaseBalance(int amount);
    bool placeBet(int amount);
    void clearHands();
    void addHand(const Hand& hand);
    std::vector<Hand>& getHands();
    const std::vector<Hand>& getHands() const;
    bool canSplit() const;
    bool performSplit();
    std::vector<int> getBets() const;
    void setBetForHand(size_t index, int amount);
private:
    int balance;
    int currentBet;
    std::vector<Hand> hands;
    std::vector<int> bets;
};

#endif
