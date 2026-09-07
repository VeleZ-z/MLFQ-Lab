#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "domain/Metrics.h"
#include "domain/MlfqConfig.h"
#include "domain/MlfqEngine.h"
#include "domain/MlfqPolicy.h"
#include "domain/Process.h"
#include "domain/Queue.h"
#include "infrastructure/CsvWriter.h"
#include "infrastructure/ProcessFactory.h"

namespace {

void printUsage(const char* prog) {
    std::cerr << "Uso: " << prog << " [archivo_procesos.csv]\n"
              << "  Sin argumentos: ejecuta el escenario de prueba por defecto.\n"
              << "  Con argumento : lee los procesos desde archivo CSV\n"
              << "                  (columnas PID,Arrival,Burst).\n";
}

} // namespace

int main(int argc, char* argv[]) {
    std::vector<domain::Process> processes;
    std::string input_path;

    try {
        if (argc > 2) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }

        if (argc == 2) {
            input_path = argv[1];
            processes = infrastructure::ProcessFactory::fromCsv(input_path);
        } else {
            processes = infrastructure::ProcessFactory::defaultScenario();
        }

        // Construccion de las colas segun la configuracion del MLFQ.
        domain::MlfqConfig config = domain::defaultMlfqConfig();
        std::vector<std::unique_ptr<domain::IQueue>> queues;
        for (int quantum : config.quantums) {
            queues.push_back(std::make_unique<domain::FifoQueue>(quantum));
        }

        auto policy = std::make_unique<domain::MlfqPolicy>();
        domain::MlfqEngine engine(std::move(policy), std::move(queues), config);

        domain::SimulationResult result = engine.run(std::move(processes));

        infrastructure::CsvWriter::write("results.csv", result.processes);

        // Resumen por consola.
        domain::MetricsSummary summary =
            domain::summarize(result.processes);

        std::cout << "Simulacion completada. Resultados exportados a results.csv\n";
        std::cout << "Promedios:\n"
                  << "  Response:   " << summary.avg_response << "\n"
                  << "  Turnaround: " << summary.avg_turnaround << "\n"
                  << "  Waiting:    " << summary.avg_waiting << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}