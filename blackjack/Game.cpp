#include "Game.h"
#include <string>
#include <vector>

Game::Game(int initialBalance)
    : deck(),
    player(initialBalance),
    dealer(),
    history(),
    state(RoundState::Idle),
    lastResult(""),
    activeHandIndex(0)
{
}

int Game::getBalance() const
{
    return player.getBalance();
}

int Game::getCurrentBet() const
{
    return player.getCurrentBet();
}

RoundState Game::getState() const
{
    return state;
}

std::string Game::getLastResult() const
{
    return lastResult;
}

std::vector<int> Game::getPlayerBets() const
{
    return player.getBets();
}

std::string Game::applyPromoCode(const std::string& code)
{
    if (code == "FREE100")
    {
        player.increaseBalance(100);
        lastResult = "Promo applied: +100$, balance: $" + std::to_string(player.getBalance());
        history.addRecord(lastResult);
        return lastResult;
    }

    if (code == "404")
    {
        lastResult = "Exiting game...";
        history.addRecord(lastResult);
        return lastResult;
    }

    if (code == "777")
    {
        player.increaseBalance(500);
        lastResult = "Promo applied: +500$, balance: $" + std::to_string(player.getBalance());
        history.addRecord(lastResult);
        return lastResult;
    }

    if (code == "666")
    {
        player.decreaseBalance(500);
        lastResult = "Promo applied: -500$, balance: $" + std::to_string(player.getBalance());
        history.addRecord(lastResult);
        return lastResult;
    }

    lastResult = "Invalid promo code";
    history.addRecord(lastResult);
    return lastResult;
}

void Game::resetRound()
{
    player.clearHands();
    dealer.clearHand();
    state = RoundState::Idle;
    activeHandIndex = 0;
    lastResult.clear();
}

void Game::ensureDeck()
{
}

bool Game::startRound(int bet)
{
    if (state == RoundState::InProgress)
    {
        lastResult = "Round already in progress.";
        history.addRecord(lastResult);
        return false;
    }

    if (bet <= 0)
    {
        lastResult = "Bet must be greater than zero.";
        history.addRecord(lastResult);
        return false;
    }

    if (bet > player.getBalance())
    {
        lastResult = "Insufficient balance.";
        history.addRecord(lastResult);
        return false;
    }

    resetRound();

    if (!player.placeBet(bet))
    {
        lastResult = "Bet placement failed.";
        history.addRecord(lastResult);
        return false;
    }

    ensureDeck();
    dealInitialCards();

    state = RoundState::InProgress;
    activeHandIndex = 0;

    if (hintsEnabled)
    {
        int dealerUp = dealer.getUpCardValue();
        const auto& hands = player.getHands();
        if (!hands.empty())
        {
            lastResult = Settings::getHint(hands[0], dealerUp);
            history.addRecord(lastResult);
        }
    }
    else
    {
        lastResult = "Round started";
        history.addRecord(lastResult);
    }

    return true;
}



void Game::dealInitialCards()
{
    dealer.clearHand();

    auto& hands = player.getHands();
    hands.clear();

    Hand firstHand;
    firstHand.addCard(deck.drawCard());
    firstHand.addCard(deck.drawCard());
    player.addHand(firstHand);

    player.setBetForHand(0, player.getCurrentBet());

    dealer.addCard(deck.drawCard());
    dealer.addCard(deck.drawCard());
}



bool Game::hit()
{
    if (state != RoundState::InProgress)
    {
        lastResult = "Round is already finished. Actions are disabled.";
        history.addRecord(lastResult);
        return false;
    }

    auto& hands = player.getHands();
    if (activeHandIndex >= hands.size())
        return false;

    hands[activeHandIndex].addCard(deck.drawCard());

    if (hands[activeHandIndex].isBust())
    {
        auto bets = player.getBets();
        int bet = (activeHandIndex < bets.size()) ? bets[activeHandIndex] : 0;

        lastResult = "Bust: -" + std::to_string(bet) +
            ", balance: $" + std::to_string(player.getBalance());
        history.addRecord(lastResult);

        ++activeHandIndex;
        if (activeHandIndex >= hands.size())
            processDealerAndResolve();
    }

    return true;
}


bool Game::stand()
{
    if (state != RoundState::InProgress)
    {
        lastResult = "Round is already finished. Actions are disabled.";
        history.addRecord(lastResult);
        return false;
    }

    ++activeHandIndex;
    if (activeHandIndex >= player.getHands().size())
        processDealerAndResolve();

    return true;
}

bool Game::doubleDown()
{
    if (state != RoundState::InProgress)
    {
        lastResult = "Round is already finished. Actions are disabled.";
        history.addRecord(lastResult);
        return false;
    }

    auto bets = player.getBets();
    if (activeHandIndex >= bets.size())
        return false;

    int bet = bets[activeHandIndex];
    if (bet > player.getBalance())
        return false;

    player.decreaseBalance(bet);
    player.setBetForHand(activeHandIndex, bet * 2);

    auto& hands = player.getHands();
    hands[activeHandIndex].addCard(deck.drawCard());

    if (hands[activeHandIndex].isBust())
    {
        lastResult = "Bust after double: -" + std::to_string(bet * 2) +
            ", balance: $" + std::to_string(player.getBalance());
        history.addRecord(lastResult);
    }

    ++activeHandIndex;
    if (activeHandIndex >= hands.size())
        processDealerAndResolve();

    return true;
}

bool Game::split()
{
    if (state != RoundState::InProgress)
    {
        lastResult = "Round is already finished. Actions are disabled.";
        history.addRecord(lastResult);
        return false;
    }

    if (!player.canSplit())
        return false;

    if (!player.performSplit())
        return false;

    auto& hands = player.getHands();
    if (hands.size() < 2)
        return false;

    hands[0].addCard(deck.drawCard());
    hands[1].addCard(deck.drawCard());

    auto bets = player.getBets();
    if (!bets.empty())
    {
        int bet0 = bets[0];
        if (bet0 <= player.getBalance())
        {
            player.decreaseBalance(bet0);
            player.setBetForHand(1, bet0);
        }
    }

    activeHandIndex = 0;
    return true;
}



void Game::processDealerAndResolve()
{
    dealer.playTurn(deck);

    auto& hands = player.getHands();
    for (size_t i = 0; i < hands.size(); ++i)
        resolveHand(i);

    state = RoundState::Finished;
}

void Game::resolveHand(size_t handIndex)
{
    const auto& hands = player.getHands();
    if (handIndex >= hands.size())
        return;

    const Hand& ph = hands[handIndex];
    int pValue = ph.getValue();
    int dValue = dealer.getValue();

    auto bets = player.getBets();
    if (bets.empty() || handIndex >= bets.size())
        return;

    int bet = bets[handIndex];
    std::string outcome;

    if (ph.isBust())
    {
        outcome = "Lose: -" + std::to_string(bet);
    }
    else if (dValue > 21 || pValue > dValue)
    {
        player.increaseBalance(bet * 2);
        outcome = "Win: +" + std::to_string(bet);
    }
    else if (pValue == dValue)
    {
        player.increaseBalance(bet);
        outcome = "Push: 0";
    }
    else
    {
        outcome = "Lose: -" + std::to_string(bet);
    }

    lastResult = outcome + ", balance: $" + std::to_string(player.getBalance());
    history.addRecord(lastResult);
}

std::vector<std::string> Game::getPlayerHandsStrings() const
{
    std::vector<std::string> result;
    const auto& hands = player.getHands();

    for (size_t i = 0; i < hands.size(); ++i)
    {
        const auto& cards = hands[i].getCards();
        std::string s = "Hand " + std::to_string(i + 1) + ": ";

        for (const auto& c : cards)
        {
            s += "[" + c.toShortString() + "]";
        }

        s += "  (value " + std::to_string(hands[i].getValue()) + ")";

        result.push_back(s);
    }

    return result;
}


std::string Game::getDealerString(bool hideHoleCard) const
{
    return dealer.toString(hideHoleCard);
}
