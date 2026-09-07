#ifndef MLFQ_CSV_WRITER_H
#define MLFQ_CSV_WRITER_H

#include <string>
#include <vector>

#include "domain/Process.h"

namespace infrastructure {

// Escribe las metricas de los procesos a un archivo CSV con el formato
// exigido por el laboratorio:
//   PID,Arrival,Burst,Start,Finish,Response,Turnaround,Waiting
class CsvWriter {
public:
    static void write(const std::string& path,
                      const std::vector<domain::Process>& processes);
};

} // namespace infrastructure

#endif