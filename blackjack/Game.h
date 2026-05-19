#ifndef GAME_H
#define GAME_H

#include "Deck.h"
#include "Player.h"
#include "Dealer.h"
#include "History.h"
#include <string>
#include <vector>

enum class RoundState
{
    Idle,
    InProgress,
    Finished
};

class Game
{
public:
    Game(int initialBalance = 1000);
    int getBalance() const;
    int getCurrentBet() const;
    bool startRound(int bet);
    bool hit();
    bool stand();
    bool doubleDown();
    bool split();
    void processDealerAndResolve();
    std::vector<std::string> getPlayerHandsStrings() const;
    std::string getDealerString(bool hideHoleCard) const;
    std::string getLastResult() const;
    RoundState getState() const;
    std::string applyPromoCode(const std::string& code);
    std::vector<int> getPlayerBets() const;
    void resetRound();
private:
    Deck deck;
    Player player;
    Dealer dealer;
    History history;
    RoundState state;
    std::string lastResult;
    size_t activeHandIndex;
    void dealInitialCards();
    void resolveHand(size_t handIndex);
    void ensureDeck();
};

#endif