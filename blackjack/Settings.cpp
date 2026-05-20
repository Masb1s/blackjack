#include "Settings.h"

bool Settings::hints = false;

void Settings::enableHints(bool v)
{
    hints = v;
}

bool Settings::hintsEnabled()
{
    return hints;
}

std::string Settings::getHint(const Hand& ph, int dealerUp)
{
    int p = ph.getValue();

    if (p <= 8)
        return "Hint: Hit";

    if (p == 9)
        return (dealerUp >= 3 && dealerUp <= 6) ? "Hint: Double" : "Hint: Hit";

    if (p == 10)
        return (dealerUp <= 9) ? "Hint: Double" : "Hint: Hit";

    if (p == 11)
        return "Hint: Double";

    if (p == 12)
        return (dealerUp >= 4 && dealerUp <= 6) ? "Hint: Stand" : "Hint: Hit";

    if (p >= 13 && p <= 16)
        return (dealerUp >= 2 && dealerUp <= 6) ? "Hint: Stand" : "Hint: Hit";

    return "Hint: Stand";
}