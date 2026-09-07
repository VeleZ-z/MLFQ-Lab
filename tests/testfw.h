#ifndef MLFQ_TEST_H
#define MLFQ_TEST_H

#include <cmath>
#include <cstdio>
#include <string>

namespace testfw {

// Micro-framework de pruebas sin dependencias externas.
struct Context {
    int checks = 0;
    int failures = 0;

    void expect(bool condition, const std::string& message) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "  [FALLO] %s\n", message.c_str());
        }
    }

    void expectNear(double actual, double expected,
                    const std::string& message) {
        ++checks;
        if (std::fabs(actual - expected) > 1e-9) {
            ++failures;
            std::fprintf(stderr, "  [FALLO] %s (esperado %.6f, obtenido %.6f)\n",
                         message.c_str(), expected, actual);
        }
    }

    int summary() const {
        std::printf("%d verificaciones, %d fallos\n", checks, failures);
        return failures == 0 ? 0 : 1;
    }
};

} // namespace testfw

#endif