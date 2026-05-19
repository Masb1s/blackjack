#ifndef HISTORY_H
#define HISTORY_H

#include <vector>
#include <string>

class History
{
public:
    History();
    void addRecord(const std::string& record);
    std::vector<std::string> getRecords() const;
    std::string getAllRecords() const;
private:
    std::vector<std::string> records;
};

#endif
