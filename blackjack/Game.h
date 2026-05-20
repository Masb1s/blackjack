#ifndef GAME_H
#define GAME_H

#include "Deck.h"
#include "Player.h"
#include "Dealer.h"
#include "History.h"
#include "Settings.h"
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
    void resetRound();
    void processDealerAndResolve();

    std::vector<std::string> getPlayerHandsStrings() const;
    std::string getDealerString(bool hideHoleCard) const;
    std::string getLastResult() const;
    RoundState getState() const;
    std::vector<int> getPlayerBets() const;

    std::string applyPromoCode(const std::string& code);

    void enableHints(bool v) { hintsEnabled = v; }
    bool areHintsEnabled() const { return hintsEnabled; }

    const Hand& getPlayerHand(size_t index) const { return player.getHands()[index]; }
    int getDealerUpCardValue() const { return dealer.getUpCardValue(); }

private:
    Deck deck;
    Player player;
    Dealer dealer;
    History history;

    RoundState state;
    std::string lastResult;
    size_t activeHandIndex;

    bool hintsEnabled = false;


    void dealInitialCards();
    void resolveHand(size_t handIndex);
    void ensureDeck();
};

#endif
