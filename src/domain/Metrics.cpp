#include "domain/Metrics.h"

namespace domain {

Metrics computeMetrics(const Process& p) {
    Metrics m{};
    m.response_time = p.first_response_time - p.arrival_time;
    m.turnaround_time = p.finish_time - p.arrival_time;
    m.waiting_time = m.turnaround_time - p.burst_time;
    return m;
}

MetricsSummary summarize(const std::vector<Process>& processes) {
    if (processes.empty()) {
        return MetricsSummary{0.0, 0.0, 0.0};
    }
    double sum_response = 0.0;
    double sum_turnaround = 0.0;
    double sum_waiting = 0.0;
    for (const Process& p : processes) {
        Metrics m = computeMetrics(p);
        sum_response += m.response_time;
        sum_turnaround += m.turnaround_time;
        sum_waiting += m.waiting_time;
    }
    double n = static_cast<double>(processes.size());
    return MetricsSummary{sum_response / n, sum_turnaround / n, sum_waiting / n};
}

} // namespace domain