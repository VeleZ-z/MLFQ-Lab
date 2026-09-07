#ifndef MLFQ_MLFQ_POLICY_H
#define MLFQ_MLFQ_POLICY_H

#include "domain/SchedulerPolicy.h"

namespace domain {

// Implementacion concreta de la politica MLFQ.
// Regla de seleccion: ejecutar siempre el proceso en la cola de mayor
// prioridad (menor indice) no vacia.
class MlfqPolicy final : public ISchedulingPolicy {
public:
    int selectNext(const std::vector<IQueue*>& queues,
                   std::vector<Process>& processes) override;
};

} // namespace domain

#endif