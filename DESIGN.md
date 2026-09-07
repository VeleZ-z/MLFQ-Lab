# Simulador MLFQ — Diseño y Decisiones

## Resumen

Simulador de planificador **Multi-Level Feedback Queue (MLFQ)** en C++17, con
tiempo discreto (ciclos de reloj), 3 colas con quantums `{2, 4, 8}`, priority
boost configurable (por defecto cada 20 ciclos) y exportación de métricas a
`results.csv`.

```
make        # compila ./mlfq
make run    # ejecuta el escenario por defecto del enunciado
./mlfq procesos.csv   # carga una carga de trabajo desde CSV (PID,Arrival,Burst)
make test   # compila y ejecuta las pruebas unitarias
make clean
```

## Estructura y capas (arquitectura limpia simplificada)

```
+-------------------------------------------------------------+
| src/main.cpp          (entrada: CLI, wiring, manejo de      |
|                        errores de nivel aplicacion)         |
+-------------------+-----------------------------------------+
                    | depende de
        +-----------v------------+      +----------------------+
        |  include/infrastructure |      |  include/domain      |
        |  ProcessFactory (CSV)   |      |  Process, Queue,     |
        |  CsvWriter (resultados) |      |  MlfqEngine, Metrics |
        +-----------+------------+      |  MlfqPolicy, Config  |
                    |                   +----------^-----------+
                    +------ ambas producen/consumen Process (dominio)
```

- **Dominio** (`include/domain`, `src/domain`): entidades y reglas del
  scheduler. **No importa nada de infraestructura** ni de la STL de
  archivos (`<fstream>`): los procesos entran y salen como
  `std::vector<Process>`. Es el núcleo testeable.
- **Infraestructura** (`include/infrastructure`, `src/infrastructure`):
  lectura de la carga de trabajo desde CSV y escritura de `results.csv`.
  Depende del dominio, nunca al revés (regla de dependencia).
- **Aplicación** (`src/main.cpp`): parseo de argumentos, ensamblado de la
  política/colas/config y manejo de errores con mensajes claros.

## Clases y responsabilidades (SRP)

| Unidad | Responsabilidad única |
|---|---|
| `Process` / `makeProcess` | Estado de un proceso e invariantes básicas |
| `IQueue` / `FifoQueue` | Abstracción de cola FIFO y su implementación |
| `ISchedulingPolicy` / `MlfqPolicy` | Elegir de qué cola sale el siguiente proceso |
| `MlfqEngine` | Avanzar el reloj, mover procesos entre estados y colas, boost |
| `Metrics` / `summarize` | Cálculo puro de response/turnaround/waiting |
| `ProcessFactory` | Crear procesos desde CSV o escenario por defecto |
| `CsvWriter` | Serializar métricas a `results.csv` |

## Principios de diseño aplicados

- **SRP**: cada clase hace una sola cosa (ver tabla). El cálculo de métricas
  está en funciones puras separadas del motor, lo que permite probarlas sin
  ejecutar la simulación.
- **OCP**: añadir una nueva cola solo exige cambiar `MlfqConfig`
  (`quantums.push_back(...)`); el motor itera sobre `raw_queues_.size()` sin
  conocer el número de niveles. Una nueva regla de selección se añade como
  otra implementación de `ISchedulingPolicy` sin tocar `MlfqEngine`.
- **DIP**: `MlfqEngine` depende de las abstracciones `ISchedulingPolicy` e
  `IQueue`, no de `FifoQueue` ni `MlfqPolicy`; las instancias concretas se
  inyectan por el constructor desde `main` (inyección de dependencias manual).
- **Encapsulamiento**: el estado interno de las colas no se expone (solo
  `push/front/pop/empty/size`); los cambios de estado del proceso solo los
  realiza el motor.

## Patrones de diseño (justificación)

- **Strategy** (`ISchedulingPolicy`): la regla de selección del MLFQ se
  encapsula tras una interfaz para poder intercambiarla (p. ej. por FCFS o RR
  puro) sin modificar el motor. Justificación: el enunciado pide OCP y el
  curso estudia varias políticas de planificación; es el punto natural de
  variación del sistema.
- **Factory** (`ProcessFactory`): centraliza la creación de `Process` desde
  dos fuentes (escenario por defecto, archivo CSV), incluyendo la validación
  de entrada. Sin ella, `main` mezclaría parsing, validación y wiring.
- **State (ligero)**: los estados del proceso se modelan con el enum
  `ProcessState{NEW, READY, RUNNING, TERMINATED}` y transiciones explícitas en
  el motor. No se usó el patrón State de GoF completo (una clase por estado)
  porque las transiciones son pocas y todas las ejecuta `MlfqEngine`; una
  jerarquía de clases de estado habría sido sobre-ingeniería.

**No se usó Observer**: registrar eventos de simulación no aporta al
entregable y añadiría acoplamiento; se privilegió la simplicidad.

## Decisiones de implementación relevantes

1. **Simulación por ciclos con conservación de CPU**: el avance es de 1 ciclo
   por iteración, pero un proceso que no ha agotado su quantum **conserva la
   CPU** (`running_idx`). Esto modela la semántica real de Round Robin:
   la expropiación ocurre por timer interrupt al final del quantum, no en
   cada ciclo. Un diseño ingenuo "seleccionar cada ciclo" rompería Round
   Robin dentro de cada cola.
2. **Selección extrae de la cola**: cuando la política elige un proceso, este
   sale de su cola mientras corre y re-ingresa (al frente de su cola, o a la
   cola inferior) al ceder la CPU. Así, `front()` siempre refleja trabajo
   realmente disponible.
3. **Demotion solo por quantum agotado**: si el proceso termina (incluso
   justo al finalizar su quantum), no baja de nivel, como indica el enunciado.
4. **Priority boost en frontera de ciclo**: vacía todas las colas, re-encola
   los READY en Q0 **en orden de PID** (determinismo) y reinicia el quantum
   restante de todos —incluido el que está corriendo— al quantum completo de
   Q0. Se hace así para que el boost sea un reseteo completo y predecible.
5. **Procesos indexados por posición, colas por PID**: las colas guardan PIDs
   (visión del scheduler) y el motor los resuelve a índices con una búsqueda
   lineal (N es pequeño en un simulador didáctico; un `unordered_map` no
   cambiaría la complejidad real de forma relevante).
6. **Manejo de errores**: la fábrica lanza `std::runtime_error` con línea y
   causa exactas (burst negativo, columnas faltantes, archivo inexistente,
   valores no numéricos); `main` los captura y sale con mensaje claro y código
   de error. No hay fallos silenciosos.

## Pruebas

`tests/test_main.cpp` (framework mínimo propio, sin dependencias):

- **Métricas**: fórmulas de response/turnaround/waiting y promedios.
- **Demotion**: un proceso que agota el quantum de Q0 termina en Q1.
- **No demotion**: un proceso que termina antes del quantum permanece en Q0.
- **Boost anti-starvation**: con boost frecuente, el proceso corto termina
  pronto aunque coexist con uno largo.
- **Prioridad estricta**: el proceso de mayor prioridad termina primero.

## Análisis (preguntas del enunciado)

- **¿Qué ocurre si el boost es muy frecuente?** El MLFQ degenera hacia un
  Round Robin con quantum pequeño en Q0: los procesos apenas llegan a bajar
  de nivel antes de ser devueltos a Q0, se pierde la "memoria" del
  comportamiento (la demotion deja de diferenciar procesos CPU-bound de
  interactivos) y aumenta el costo de cambio de contexto por preempciones
  frecuentes.
- **¿Qué ocurre si no existe boost?** Los procesos largos llegan a Q2 y
  pueden sufrir *starvation* si continuamente llegan trabajos nuevos a las
  colas superiores: Q0/Q1 nunca se vacían y Q2 no llega a ejecutarse. La
  prueba `priorityBoostPreventsStarvation` evidencia el efecto contrario.
- **¿Cómo afecta un quantum pequeño en la cola de mayor prioridad?** Mejora
  el *response time* (los procesos nuevos obtienen CPU rápido), pero
  incrementa el overhead de cambio de contexto y hace que incluso procesos
  cortos caigan a colas inferiores, penalizando trabajos medianos. Es el
  compromiso clásico entre interactividad y eficiencia.
- **¿Puede haber starvation?** Sí, sin boost: un flujo constante de procesos
  en colas de alta prioridad posterga indefinidamente a los de Q2. El
  priority boost periódico garantiza que todo proceso vuelva
  periódicamente a Q0, acotando el tiempo máximo sin CPU.
