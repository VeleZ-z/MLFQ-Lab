#ifndef MLFQ_METRICS_H
#define MLFQ_METRICS_H

#include <vector>

#include "domain/Process.h"

namespace domain {

// Las tres metricas exigidas por el laboratorio, calculadas a partir del
// estado final de cada proceso. Se aislaron en funciones puras para poder
// probarlas unitariamente sin ejecutar la simulacion completa.
struct Metrics {
    int response_time;
    int turnaround_time;
    int waiting_time;
};

Metrics computeMetrics(const Process& p);

// Promedios de las metricas sobre un conjunto de procesos.
struct MetricsSummary {
    double avg_response;
    double avg_turnaround;
    double avg_waiting;
};

MetricsSummary summarize(const std::vector<Process>& processes);

} // namespace domain

#endif