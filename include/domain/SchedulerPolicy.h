#ifndef MLFQ_SCHEDULER_POLICY_H
#define MLFQ_SCHEDULER_POLICY_H

#include <vector>

#include "domain/Queue.h"

namespace domain {

struct Process;

// Strategy: encapsula la politica de scheduling detras de una interfaz.
// Permite intercambiar MLFQ por FCFS/RR sin tocar el motor de simulacion.
class ISchedulingPolicy {
public:
    virtual ~ISchedulingPolicy() = default;

    // Aplica un paso de la politica. `queues` son las colas del sistema y
    // `processes` el estado de todos los procesos. Retorna el pid del proceso
    // elegido para ejecutar el siguiente ciclo, o -1 si no hay trabajo listo.
    virtual int selectNext(const std::vector<IQueue*>& queues,
                           std::vector<Process>& processes) = 0;
};

} // namespace domain

#endif