#include "domain/Process.h"

namespace domain {

Process makeProcess(int pid, int arrival_time, int burst_time) {
    Process p{};
    p.pid = pid;
    p.arrival_time = arrival_time;
    p.burst_time = burst_time;
    p.remaining_time = burst_time;
    p.start_time = -1;
    p.finish_time = -1;
    p.first_response_time = -1;
    p.current_queue = 0;
    p.state = ProcessState::NEW;
    return p;
}

} // namespace domain