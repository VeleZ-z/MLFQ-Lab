#ifndef MLFQ_PROCESS_FACTORY_H
#define MLFQ_PROCESS_FACTORY_H

#include <string>
#include <vector>

#include "domain/Process.h"

namespace infrastructure {

// Factory: centraliza la creacion de procesos desde distintas fuentes
// (escenario por defecto o archivo CSV). Anadir una nueva fuente solo
// implica agregar un metodo aqui, sin tocar el dominio.
class ProcessFactory {
public:
    // Procesos del escenario de prueba sugerido por el laboratorio.
    static std::vector<domain::Process> defaultScenario();

    // Lee procesos desde un CSV con columnas: PID, Arrival, Burst.
    // Lanza std::runtime_error con un mensaje claro si el archivo esta mal
    // formado (valores faltantes, burst negativo, etc.).
    static std::vector<domain::Process> fromCsv(const std::string& path);
};

} // namespace infrastructure

#endif