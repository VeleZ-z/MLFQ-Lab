#ifndef MLFQ_CONFIG_H
#define MLFQ_CONFIG_H

#include <vector>

namespace domain {

// Parametros de configuracion del MLFQ, agrupados para poder variarlos
// (OCP: anadir niveles u otra regla de demotion no exige tocar el motor).
struct MlfqConfig {
    // Quantums por nivel, indexado por nivel (0 = mayor prioridad).
    std::vector<int> quantums;
    // Numero de ciclos entre priority boosts.
    int boost_interval;
};

// Configuracion del laboratorio: Q0=2, Q1=4, Q2=8, boost cada 20 ciclos.
inline MlfqConfig defaultMlfqConfig() {
    return MlfqConfig{{2, 4, 8}, 20};
}

} // namespace domain

#endif