#include "infrastructure/ProcessFactory.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace infrastructure {

std::vector<domain::Process> ProcessFactory::defaultScenario() {
    std::vector<domain::Process> procs;
    procs.push_back(domain::makeProcess(1, 0, 8));
    procs.push_back(domain::makeProcess(2, 1, 4));
    procs.push_back(domain::makeProcess(3, 2, 9));
    procs.push_back(domain::makeProcess(4, 3, 5));
    return procs;
}

std::vector<domain::Process> ProcessFactory::fromCsv(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo de procesos: " + path);
    }

    std::vector<domain::Process> procs;
    std::string line;
    int line_number = 0;

    while (std::getline(in, line)) {
        ++line_number;

        // Ignorar lineas vacias.
        if (line.empty()) {
            continue;
        }
        // Ignorar el encabezado si la primera linea contiene texto no numerico.
        if (line_number == 1 && line.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") != std::string::npos) {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        int values[3];
        int count = 0;

        while (std::getline(ss, token, ',') && count < 3) {
            try {
                values[count] = std::stoi(token);
            } catch (const std::exception&) {
                throw std::runtime_error(
                    "Valor no numerico en la linea " + std::to_string(line_number) +
                    " del archivo " + path);
            }
            ++count;
        }

        if (count < 3) {
            throw std::runtime_error(
                "Linea " + std::to_string(line_number) +
                " mal formada: se esperaban 3 columnas (PID,Arrival,Burst).");
        }

        int pid = values[0];
        int arrival = values[1];
        int burst = values[2];

        if (burst < 0) {
            throw std::runtime_error(
                "Burst negativo para el proceso " + std::to_string(pid) +
                " (linea " + std::to_string(line_number) + ").");
        }
        if (arrival < 0) {
            throw std::runtime_error(
                "Arrival negativo para el proceso " + std::to_string(pid) +
                " (linea " + std::to_string(line_number) + ").");
        }

        procs.push_back(domain::makeProcess(pid, arrival, burst));
    }

    if (procs.empty()) {
        throw std::runtime_error("El archivo no contiene procesos validos: " + path);
    }

    return procs;
}

} // namespace infrastructure