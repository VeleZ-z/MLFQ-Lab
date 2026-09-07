#include "domain/MlfqEngine.h"

namespace domain {

namespace {

// Localiza el indice del proceso cuyo pid coincide con `pid`. Devuelve -1 si
// no lo encuentra (no deberia ocurrir: las colas solo contienen pids de
// procesos del vector).
int indexOf(const std::vector<Process>& processes, int pid) {
    for (std::size_t i = 0; i < processes.size(); ++i) {
        if (processes[i].pid == pid) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace

MlfqEngine::MlfqEngine(std::unique_ptr<ISchedulingPolicy> policy,
                       std::vector<std::unique_ptr<IQueue>>&& queues,
                       MlfqConfig config)
    : policy_(std::move(policy)),
      queues_(std::move(queues)),
      config_(config) {
    for (auto& q : queues_) {
        raw_queues_.push_back(q.get());
    }
}

void MlfqEngine::enqueueArrivals(std::vector<Process>& processes, int clock) {
    // Todo proceso cuya hora de llegada coincida con el reloj actual pasa a
    // READY y se encola en su cola actual (inicialmente la de mayor prioridad).
    for (Process& p : processes) {
        if (p.state == ProcessState::NEW && p.arrival_time == clock) {
            p.state = ProcessState::READY;
            if (p.current_queue >= 0 &&
                static_cast<std::size_t>(p.current_queue) < raw_queues_.size()) {
                raw_queues_[p.current_queue]->push(p.pid);
            }
        }
    }
}

int MlfqEngine::selectNext(std::vector<Process>& processes) {
    return policy_->selectNext(raw_queues_, processes);
}

SimulationResult MlfqEngine::run(std::vector<Process> processes) {
    SimulationResult result;
    const int total = static_cast<int>(processes.size());
    int clock = 0;
    int finished = 0;
    int last_boost = 0;

    // Quantum restante de cada proceso en su nivel actual. Solo es distinto
    // de cero para el proceso que tiene la CPU: al llegar llega con quantum
    // completo, al bajar de nivel se reinicia para su proxima ejecucion.
    std::vector<int> quantum_left(processes.size(), 0);

    // Indice del proceso que tiene la CPU, o -1 si la CPU esta libre. Un
    // proceso conserva la CPU hasta terminar o agotar su quantum: esta es la
    // semantica de Round Robin (preemption solo via timer interrupt), no una
    // re-asignacion en cada ciclo.
    int running_idx = -1;

    while (finished < total) {
        enqueueArrivals(processes, clock);

        // Priority boost periodico. Se aplica en fronteras de ciclo y, por
        // decision de diseno documentada en DESIGN.md, no interrumpe el
        // intervalo actual del proceso en CPU: solo reinicia su nivel y su
        // quantum para que continue con el quantum completo de Q0.
        if (config_.boost_interval > 0 && clock > 0 &&
            clock - last_boost >= config_.boost_interval) {
            for (std::size_t level = 0; level < raw_queues_.size(); ++level) {
                while (!raw_queues_[level]->empty()) {
                    raw_queues_[level]->pop();
                }
            }
            for (std::size_t i = 0; i < processes.size(); ++i) {
                if (processes[i].state == ProcessState::READY ||
                    processes[i].state == ProcessState::RUNNING) {
                    processes[i].current_queue = 0;
                    quantum_left[i] = 0; // quantum completo al re-entrar a Q0
                }
                if (processes[i].state == ProcessState::READY) {
                    raw_queues_[0]->push(processes[i].pid);
                }
            }
            last_boost = clock;
        }

        // Si la CPU esta libre, la politica elige el siguiente proceso.
        if (running_idx == -1) {
            int pid = selectNext(processes);
            if (pid == -1) {
                // Nadie listo: CPU ociosa un ciclo.
                result.timeline.push_back(-1);
                ++clock;
                continue;
            }
            running_idx = indexOf(processes, pid);
            if (running_idx == -1) { // defensivo, no deberia ocurrir
                result.timeline.push_back(-1);
                ++clock;
                continue;
            }
        }

        Process& p = processes[running_idx];

        // Quantum completo al entrar a ejecutar (tambien cubre el caso del
        // proceso que estaba corriendo cuando ocurrio un boost).
        if (quantum_left[running_idx] == 0) {
            quantum_left[running_idx] = config_.quantums[p.current_queue];
        }

        // Response time y start time: primer instante en que recibe CPU.
        if (!p.has_responded()) {
            p.first_response_time = clock;
        }
        if (!p.has_started()) {
            p.start_time = clock;
        }
        p.state = ProcessState::RUNNING;

        // Avanza un ciclo de reloj con este proceso en la CPU.
        result.timeline.push_back(p.pid);
        --p.remaining_time;
        --quantum_left[running_idx];
        ++clock;

        if (p.remaining_time == 0) {
            // Termino (antes o justo al agotar el quantum): no se demueve.
            p.state = ProcessState::TERMINATED;
            p.finish_time = clock;
            quantum_left[running_idx] = 0;
            running_idx = -1;
            ++finished;
        } else if (quantum_left[running_idx] == 0) {
            // Agoto su quantum sin terminar: demotion a la cola inferior
            // (si ya esta en la ultima, se queda en ella, Round Robin puro).
            p.state = ProcessState::READY;
            if (p.current_queue + 1 < static_cast<int>(raw_queues_.size())) {
                ++p.current_queue;
            }
            raw_queues_[p.current_queue]->push(p.pid);
            running_idx = -1;
        }
        // Si aun le queda quantum, conserva la CPU (sigue en estado RUNNING).
    }

    result.processes = std::move(processes);
    return result;
}

} // namespace domain