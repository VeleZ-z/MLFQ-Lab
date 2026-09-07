# Simulador de Planificador MLFQ (Multi-Level Feedback Queue)

Este proyecto implementa un simulador de planificador de procesos basado en la política **Multi-Level Feedback Queue (MLFQ)** desarrollado en **C++17**. Diseñado bajo principios de **Arquitectura Limpia**, **SOLID** y patrones de diseño de software para garantizar un código modular, mantenible, extensible y testeable.

El simulador avanza mediante ciclos de reloj discreto, emula la ejecución de procesos bajo 3 niveles de prioridad con quantums diferenciados, realiza descensos de prioridad (*demotion*), ejecuta reajustes periódicos (*priority boost*) para evitar inanición (*starvation*), calcula métricas clave y exporta los resultados en un formato estándar CSV (`results.csv`).

---

##  Características Principales

*   **Planificación MLFQ de 3 Niveles**:
    *   **Q0 (Prioridad Alta)**: Cola FIFO / Round Robin con quantum de **2 ciclos**.
    *   **Q1 (Prioridad Media)**: Cola FIFO / Round Robin con quantum de **4 ciclos**.
    *   **Q2 (Prioridad Baja)**: Cola FIFO / Round Robin con quantum de **8 ciclos**.
*   **Conservación de CPU (Preempción Justa)**: Un proceso en ejecución conserva la CPU hasta completar su quantum o finalizar su ráfaga, respetando la semántica de interrupción por temporizador.
*   **Mecanismo de Democión Condicional**: Los procesos solo bajan de nivel de prioridad si consumen la totalidad de su quantum asignado. Si terminan antes, no sufren democión.
*   **Priority Boost Periódico**: Cada intervalo configurable (por defecto cada $S = 20$ ciclos), todos los procesos son retornados a la cola de máxima prioridad (Q0) de manera determinista (por PID) para evitar inanición.
*   **Cálculo Automático de Métricas**:
    *   **Response Time**: Tiempo transcurrido desde la llegada hasta la primera ejecución.
    *   **Turnaround Time**: Tiempo total transcurrido desde la llegada hasta la finalización.
    *   **Waiting Time**: Tiempo total que el proceso permaneció en espera de CPU.
*   **Persistencia en CSV**: Exportación detallada de la ejecución de cada proceso en `results.csv`.

---

## Arquitectura del Sistema

El simulador sigue un enfoque de **Arquitectura Limpia Simplificada** para separar las reglas de negocio de la infraestructura y de la entrada/salida:

```text
+-------------------------------------------------------------+
|                 src/main.cpp (Aplicación)                   |
|   - Orquestación y cableado (wiring)                        |
|   - Manejo global de excepciones de nivel aplicación        |
+---------------------+---------------------------------------+
                      |
                      v depende de
+------------------------------------+   +----------------------------+
|      include/infrastructure        |   |       include/domain       |
|                                    |   |                            |
|   - ProcessFactory (CSV/Default)  |-->|   - Process & Queue        |
|   - CsvWriter (results.csv)        |   |   - MlfqEngine & MlfqPolicy|
|                                    |   |   - Metrics & MlfqConfig   |
+------------------------------------+   +----------------------------+
  Ambas consumen/producen entidades
  del dominio (Process)
```

### Capas del Proyecto

1.  **Dominio (`include/domain`, `src/domain`)**: Contiene el núcleo matemático y algorítmico del planificador. Es completamente independiente de la infraestructura, de la consola y de sistemas de archivos. Los datos entran y salen como colecciones estándar de la STL (`std::vector`). Esto garantiza una testeabilidad unitaria perfecta.
2.  **Infraestructura (`include/infrastructure`, `src/infrastructure`)**: Encargada del parsing de datos de entrada desde un archivo CSV y de serializar los resultados de la simulación en `results.csv`. Depende exclusivamente del dominio.
3.  **Aplicación (`src/main.cpp`)**: Punto de entrada de la aplicación. Realiza la inyección de dependencias manual, define la configuración global y maneja los errores mediante mensajes amigables al usuario.

---

##  Principios de Diseño Aplicados (SOLID & Co.)

*   **SRP (Single Responsibility Principle)**: Cada clase tiene una única e inequívoca responsabilidad. Por ejemplo, el motor de simulación (`MlfqEngine`) coordina el reloj y los estados del proceso, mientras que las métricas se calculan mediante funciones puras separadas en `Metrics.h` y la serialización reside en `CsvWriter.h`.
*   **OCP (Open/Closed Principle)**: Añadir un nuevo nivel de cola es tan simple como agregar un valor al vector en `MlfqConfig` sin modificar el motor del scheduler. Si se desea cambiar la lógica de selección de procesos por FCFS o Round Robin puro, solo es necesario inyectar una nueva implementación de `ISchedulingPolicy` sin alterar el código de `MlfqEngine`.
*   **DIP (Dependency Inversion Principle)**: `MlfqEngine` no depende de implementaciones de cola concretas (`FifoQueue`) ni de políticas específicas (`MlfqPolicy`). En su lugar, se comunica a través de interfaces (`IQueue`, `ISchedulingPolicy`), las cuales se inyectan dinámicamente mediante el constructor desde el entry point (`main`).
*   **Encapsulamiento**: El estado interno de los procesos solo es editable bajo las reglas estrictas de transiciones del motor de simulación. Las colas exponen un comportamiento opaco (`push`, `pop`, `front`, `empty`, `size`) para evitar la manipulación directa de sus estructuras internas.
*   **DRY (Don't Repeat Yourself)**: Reutilización de lógica de encolado y transiciones de estados mediante métodos auxiliares parametrizados dentro del motor.

---

##  Patrones de Diseño Implementados

*   **Strategy Pattern (`ISchedulingPolicy`)**: Encapsula el criterio de selección del proceso a planificar de manera modular. Esto permite intercambiar políticas de planificación sin tocar la lógica de simulación, algo clave para el OCP y la experimentación académica.
*   **Factory Pattern (`ProcessFactory`)**: Centraliza la carga y validación de los procesos de simulación desde dos canales de entrada (un escenario de prueba por defecto cableado y un cargador dinámico de CSV). Además, realiza la validación robusta de datos (como bursts negativos, archivos faltantes o columnas corruptas), lanzando excepciones limpias de tipo `std::runtime_error`.
*   **State Pattern (Ligero)**: Los ciclos de vida de un proceso son controlados mediante transiciones formales sobre el enum `ProcessState` (`NEW`, `READY`, `RUNNING`, `TERMINATED`). Se optó por una transición controlada por el motor en lugar de una jerarquía de clases de estado pesada (GoF State) para evitar sobre-ingeniería innecesaria.

---

## Guía de Uso del Proyecto

### Requisitos Previos
*   Compilador compatible con **C++17** (ej. `g++` v7 o superior).
*   Herramienta de automatización **Make**.

### Comandos del Makefile

El proyecto incluye un `Makefile` parametrizado para facilitar las tareas de desarrollo y compilación:

*   **Compilar el simulador**:
    ```bash
    make
    ```
    Genera el ejecutable de la simulación llamado `mlfq` en la raíz del proyecto.

*   **Ejecutar el escenario por defecto**:
    ```bash
    make run
    ```
    Compila el proyecto y ejecuta la simulación con el escenario predefinido del enunciado del laboratorio.

*   **Ejecutar pruebas unitarias**:
    ```bash
    make test
    ```
    Compila y ejecuta la batería de pruebas automatizadas mediante el framework mínimo interno (`unittest`).

*   **Limpiar el entorno**:
    ```bash
    make clean
    ```
    Elimina los archivos de compilación, objetos, ejecutables (`mlfq`, `unittest`) y el reporte `results.csv`.

---

##  Ejemplos de Ejecución

### 1. Escenario por Defecto (Estatico)
Si ejecutas el simulador sin argumentos:
```bash
./mlfq
```
Se procesará la carga predefinida del laboratorio:
*   **P1**: Arrival = 0, Burst = 8
*   **P2**: Arrival = 1, Burst = 4
*   **P3**: Arrival = 2, Burst = 9
*   **P4**: Arrival = 3, Burst = 5

**Salida en consola esperada:**
```text
Simulacion completada. Resultados exportados a results.csv
Promedios:
 Response: 1.5
 Turnaround: 15.5
 Waiting: 9
```

### 2. Carga de Trabajo Personalizada desde CSV
Puedes proveer tu propio set de datos en un archivo CSV estructurado con las columnas `PID,Arrival,Burst` (sin cabecera):
```bash
./mlfq mi_carga_de_procesos.csv
```

**Formato recomendado de entrada (`procesos.csv`):**
```csv
P1,0,10
P2,2,6
P3,4,4
```

---

##  Pruebas Unitarias

El simulador incorpora pruebas automatizadas dentro del archivo `tests/test_main.cpp` bajo un micro-framework interno (`testfw.h`), validando escenarios de comportamiento críticos:

*   `testMetricsCalculation`: Valida las fórmulas puras del cálculo de métricas de rendimiento.
*   `testProcessDemotion`: Comprueba que un proceso que agota su quantum en Q0 es descendido a Q1.
*   `testNoDemotionWhenYielding`: Asegura que un proceso que termina voluntariamente antes de expirar su quantum no baja de nivel.
*   `priorityBoostPreventsStarvation`: Comprueba que un proceso con ráfagas extensas en colas bajas no sufre de starvation al activarse el boost frecuentemente.
*   `strictPriorityScheduling`: Verifica que las colas de mayor prioridad siempre obtengan atención de CPU antes que las colas de baja prioridad.

---

##  Análisis Técnico y Preguntas del Laboratorio

### 1. ¿Qué ocurre si el boost es muy frecuente?
Si el *Priority Boost* ocurre de manera demasiado frecuente (por ejemplo, cada 2-3 ciclos), **el algoritmo MLFQ degenera hacia un Round Robin puro** sobre la cola de máxima prioridad (Q0). Los procesos no tienen el tiempo suficiente para descender de nivel y ser diferenciados según su comportamiento de E/S o uso de CPU. Se pierde la memoria histórica de consumo del planificador, provocando que procesos de uso intensivo de CPU compitan en igualdad de condiciones con los procesos interactivos, perdiendo el beneficio de interactividad y aumentando el costo de sobrecarga por excesivos re-encolados y cambios de contexto.

### 2. ¿Qué ocurre si no existe boost?
La ausencia del *Priority Boost* introduce el riesgo inminente de **Inanición (Starvation)** para los procesos de larga duración que se encuentran relegados en la cola de menor prioridad (Q2). Si existe un flujo constante de nuevos procesos interactivos o de corta duración que entran a Q0 o Q1, estas colas nunca se vaciarán por completo. En consecuencia, el planificador (que siempre atiende la cola de mayor prioridad disponible) nunca seleccionará los procesos de Q2, dejándolos esperando indefinidamente. El boost garantiza una cota máxima en el tiempo de espera.

### 3. ¿Cómo afecta un quantum pequeño en la cola de mayor prioridad?
Un quantum muy pequeño en Q0 (por ejemplo, 1 ciclo) **mejora drásticamente el Response Time** global del sistema, ya que cualquier proceso recién llegado obtiene CPU casi de inmediato y de forma muy interactiva. Sin embargo, tiene dos grandes contraprestaciones:
*   **Alta sobrecarga (overhead)**: Se incrementan exponencialmente las interrupciones y cambios de contexto, disminuyendo el rendimiento útil de la CPU.
*   **Democión prematura**: Procesos moderadamente cortos o interactivos agotarán su quantum de forma rápida y descenderán innecesariamente a colas inferiores, castigando su rendimiento general.

### 4. ¿Puede haber starvation?
**Sí, el starvation es un riesgo real en MLFQ** si no se implementa una regla de envejecimiento (*aging*) o un mecanismo periódico de *Priority Boost*. Dado que MLFQ da prioridad absoluta a las colas de mayor nivel, un suministro ininterrumpido de procesos en Q0 y Q1 bloqueará indefinidamente la ejecución de los procesos CPU-bound en Q2. La prueba unitaria de inanición en este simulador demuestra cómo el boost periódico mitiga este riesgo de manera efectiva.

---

##  Estructura de Directorios

```text
.
├── 3. Laboratorio final unidad 1.md  # Enunciado y requerimientos
├── DESIGN.md                         # Documentación técnica y decisiones de diseño
├── Makefile                          # Script de automatización de compilación
├── build/                            # Objetos compilados (*.o) - Autogenerado
├── include/                          # Cabeceras (.h)
│   ├── domain/                       # Core y Reglas de Negocio (Dominio)
│   │   ├── Metrics.h
│   │   ├── MlfqConfig.h
│   │   ├── MlfqEngine.h
│   │   ├── MlfqPolicy.h
│   │   ├── Process.h
│   │   ├── Queue.h
│   │   └── SchedulerPolicy.h
│   └── infrastructure/               # Acceso a Datos e I/O (Infraestructura)
│       ├── CsvWriter.h
│       └── ProcessFactory.h
├── mlfq                              # Binario ejecutable principal - Autogenerado
├── results.csv                       # Métricas exportadas de la simulación - Autogenerado
├── src/                              # Código fuente (.cpp)
│   ├── domain/                       # Implementación de reglas del scheduler
│   ├── infrastructure/               # Implementación de lectura/escritura CSV
│   └── main.cpp                      # Orquestador del simulador
├── tests/                            # Pruebas unitarias
│   ├── test_main.cpp                 # Casos de prueba automatizados
│   └── testfw.h                      # Micro-framework de aserciones de prueba
└── unittest                          # Binario de ejecución de pruebas - Autogenerado
```
