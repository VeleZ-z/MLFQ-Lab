#ifndef MLFQ_ENGINE_H
#define MLFQ_ENGINE_H

#include <memory>
#include <vector>

#include "domain/MlfqConfig.h"
#include "domain/Process.h"
#include "domain/Queue.h"
#include "domain/SchedulerPolicy.h"

namespace domain {

struct SimulationResult {
    // Procesos con sus metricas ya calculadas.
    std::vector<Process> processes;
    // Traza de ejecucion: para cada ciclo, el pid en CPU (-1 = idle).
    std::vector<int> timeline;
};

// Motor de simulacion por ciclos de reloj (tiempo discreto).
// Depende de una ISchedulingPolicy (Strategy) para la seleccion de procesos
// y de IQueue para el manejo de colas (DIP): no conoce ninguna
// implementacion concreta de cola mas alla de la interfaz.
class MlfqEngine {
public:
    MlfqEngine(std::unique_ptr<ISchedulingPolicy> policy,
               std::vector<std::unique_ptr<IQueue>>&& queues,
               MlfqConfig config);

    // Ejecuta la simulacion hasta que todos los procesos terminan.
    SimulationResult run(std::vector<Process> processes);

private:
    void enqueueArrivals(std::vector<Process>& processes, int clock);
    int  selectNext(std::vector<Process>& processes);

    std::unique_ptr<ISchedulingPolicy> policy_;
    std::vector<std::unique_ptr<IQueue>> queues_;
    MlfqConfig config_;
    std::vector<IQueue*> raw_queues_;
};

} // namespace domain

#endif