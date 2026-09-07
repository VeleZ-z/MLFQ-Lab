#ifndef MLFQ_PROCESS_H
#define MLFQ_PROCESS_H

namespace domain {

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    TERMINATED
};

// En el modelo MLFQ no hay estado de bloqueo: los procesos son CPU-bound.
// Aun asi mantenemos la maquina de estados completa, alineada con la
// abstraccion de proceso vista en el curso (NEW/READY/RUNNING/TERMINATED),
// para que sea trivial extenderla con E/S en el futuro.
struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;

    int start_time;             // primer instante en que se puso a ejecutar
    int finish_time;            // instante en que termino
    int first_response_time;    // primer instante en que recibio CPU

    int current_queue;          // nivel de cola actual (0 = mayor prioridad)
    ProcessState state;

    bool has_started() const { return start_time >= 0; }
    bool has_responded() const { return first_response_time >= 0; }
};

// Crea un proceso nuevo listo para ejecutarse.
Process makeProcess(int pid, int arrival_time, int burst_time);

} // namespace domain

#endif