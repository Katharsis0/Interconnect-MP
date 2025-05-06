#ifndef METRICS_H
#define METRICS_H

#include <string>
#include <vector>

struct PEStats {
    int peId;
    int instructions;
    int bytesRead;
    int bytesWritten;
    int messagesSent;
    int messagesReceived;

    int bandwidth() const {
        return bytesRead + bytesWritten;
    }
};

class Metrics {
public:
    static void writeStatsToCSV(const std::vector<PEStats>& stats, const std::string& filename);
};

#endif // METRICS_H
