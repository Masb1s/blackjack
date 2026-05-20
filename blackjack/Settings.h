#pragma once
#include <string>
#include "Hand.h"

class Settings
{
public:
    static void enableHints(bool v);
    static bool hintsEnabled();
    static std::string getHint(const Hand& hand, int dealerUpCard);

private:
    static bool hints;
};
