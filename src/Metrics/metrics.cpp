#include "Metrics/metrics.h"
#include <fstream>
#include <iostream>
#include <filesystem>

void Metrics::writeStatsToCSV(const std::vector<PEStats>& stats, const std::string& filename) {
    std::cout << "Ruta absoluta: " << std::filesystem::absolute(filename) << std::endl;
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo " << filename << " para escritura.\n";

        return;
    }

    out << "PE_ID,InstructionsExecuted,BytesRead,BytesWritten,MessagesSent,MessagesReceived,BandwidthUsed(Bytes)\n";
    for (const auto& stat : stats) {
        out << stat.peId << "," << stat.instructions << "," << stat.bytesRead << ","
            << stat.bytesWritten << "," << stat.messagesSent << "," << stat.messagesReceived
            << "," << stat.bandwidth() << "\n";
    }

    out.close();
    std::cout << "Estadísticas escritas en " << filename << std::endl;
}
