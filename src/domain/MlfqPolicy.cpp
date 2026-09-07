#include "domain/MlfqPolicy.h"

namespace domain {

int MlfqPolicy::selectNext(const std::vector<IQueue*>& queues,
                           std::vector<Process>& /*processes*/) {
    // La cola de mayor prioridad es la de menor indice. Recorremos en orden
    // ascendente y elegimos la primera no vacia: asi se respeta la prioridad
    // estricta del MLFQ (una cola de mayor prioridad nunca se ve pospuesta
    // por una de menor prioridad con trabajo disponible).
    for (std::size_t level = 0; level < queues.size(); ++level) {
        IQueue* q = queues[level];
        if (q != nullptr && !q->empty()) {
            int pid = q->front();
            // Seleccionar == extraer de la cola: el proceso deja de estar
            // encolado mientras ocupa la CPU, y se re-encola (o demueve)
            // cuando cede la CPU al final de su slice.
            q->pop();
            return pid;
        }
    }
    return -1;
}

} // namespace domain