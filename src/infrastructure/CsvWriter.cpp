#include "infrastructure/CsvWriter.h"

#include <fstream>
#include <stdexcept>

#include "domain/Metrics.h"

namespace infrastructure {

void CsvWriter::write(const std::string& path,
                      const std::vector<domain::Process>& processes) {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo de resultados: " + path);
    }

    out << "PID,Arrival,Burst,Start,Finish,Response,Turnaround,Waiting\n";

    for (const domain::Process& p : processes) {
        domain::Metrics m = domain::computeMetrics(p);
        out << "P" << p.pid << ","
            << p.arrival_time << ","
            << p.burst_time << ","
            << p.start_time << ","
            << p.finish_time << ","
            << m.response_time << ","
            << m.turnaround_time << ","
            << m.waiting_time << "\n";
    }
}

} // namespace infrastructure