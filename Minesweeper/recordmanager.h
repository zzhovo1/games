#ifndef __RECORDMANAGER_H__
#define __RECORDMANAGER_H__

#include <fstream>
#include <map>
#include <string>

class RecordManager {
public:
    static int getBestTime(std::string difficulty) {
        std::map<std::string, int> records = loadAll();
        return (records.count(difficulty)) ? records[difficulty] : 999;
    }

    static void updateRecord(std::string difficulty, int seconds) {
        auto records = loadAll();
        if (records.find(difficulty) == records.end() || seconds < records[difficulty]) {
            records[difficulty] = seconds;
            saveAll(records);
        }
    }

private:
    static std::map<std::string, int> loadAll() {
        std::map<std::string, int> records;
        std::ifstream f("records.txt");
        std::string diff; 
        int time;
        while (f >> diff >> time) records[diff] = time;
        return records;
    }

    static void saveAll(const std::map<std::string, int>& records) {
        std::ofstream f("records.txt");
        for (auto const& [diff, time] : records) {
            f << diff << " " << time << "\n";
        }
    }
};

#endif // __RECORDMANAGER_H__