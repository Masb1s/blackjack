#include "History.h"
#include <sstream>

History::History()
    : records()
{
}

void History::addRecord(const std::string& record)
{
    records.push_back(record);
}

std::vector<std::string> History::getRecords() const
{
    return records;
}

std::string History::getAllRecords() const
{
    std::ostringstream oss;
    for (size_t i = 0; i < records.size(); ++i)
    {
        oss << records[i];
        if (i + 1 < records.size())
        {
            oss << "\n";
        }
    }
    return oss.str();
}
