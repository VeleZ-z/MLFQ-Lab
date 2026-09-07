#include <memory>
#include <vector>

#include "testfw.h"

#include "domain/Metrics.h"
#include "domain/MlfqConfig.h"
#include "domain/MlfqEngine.h"
#include "domain/MlfqPolicy.h"
#include "domain/Process.h"
#include "domain/Queue.h"

using namespace domain;

namespace {

MlfqEngine buildEngine(int boost_interval,
                       std::vector<int> quantums = {2, 4, 8}) {
    MlfqConfig config{quantums, boost_interval};
    std::vector<std::unique_ptr<IQueue>> queues;
    for (int q : quantums) {
        queues.push_back(std::make_unique<FifoQueue>(q));
    }
    return MlfqEngine(std::make_unique<MlfqPolicy>(),
                      std::move(queues), config);
}

} // namespace

static void testMetricsComputation(testfw::Context& ctx) {
    Process p = makeProcess(1, 4, 9);
    p.start_time = 6;
    p.first_response_time = 6;
    p.finish_time = 15;

    Metrics m = computeMetrics(p);
    ctx.expect(m.response_time == 2, "response = first_response - arrival");
    ctx.expect(m.turnaround_time == 11, "turnaround = finish - arrival");
    ctx.expect(m.waiting_time == 2, "waiting = turnaround - burst");
}

static void testMetricsSummary(testfw::Context& ctx) {
    Process p1 = makeProcess(1, 0, 2);
    p1.start_time = 0;
    p1.first_response_time = 0;
    p1.finish_time = 2;

    Process p2 = makeProcess(2, 0, 4);
    p2.start_time = 2;
    p2.first_response_time = 2;
    p2.finish_time = 6;

    MetricsSummary s = summarize({p1, p2});
    ctx.expectNear(s.avg_response, 1.0, "promedio response");
    ctx.expectNear(s.avg_turnaround, 4.0, "promedio turnaround");
    ctx.expectNear(s.avg_waiting, 1.0, "promedio waiting");
}

static void testDemotionOnFullQuantum(testfw::Context& ctx) {
    MlfqEngine engine = buildEngine(1000);
    SimulationResult r = engine.run({makeProcess(1, 0, 6)});

    ctx.expect(r.processes[0].finish_time == 6, "termina en t=6");
    ctx.expect(r.processes[0].first_response_time == 0, "responde en t=0");
    ctx.expect(r.processes[0].current_queue == 1,
               "fue demovido a Q1 tras agotar quantum");
}

static void testNoDemotionWhenFinishingEarly(testfw::Context& ctx) {
    MlfqEngine engine = buildEngine(1000);
    SimulationResult r = engine.run({makeProcess(1, 0, 1)});

    ctx.expect(r.processes[0].finish_time == 1, "termina en t=1");
    ctx.expect(r.processes[0].current_queue == 0,
               "permanece en Q0 (sin demotion)");
}

static void testPriorityBoostPreventsStarvation(testfw::Context& ctx) {
    // P1 largo (burst 10) no debe monopolizar la CPU frente a P2 corto (3).
    MlfqEngine engine = buildEngine(5);
    SimulationResult r = engine.run({makeProcess(1, 0, 10),
                                     makeProcess(2, 0, 3)});

    ctx.expect(r.processes[1].finish_time > 0, "P2 termina");
    ctx.expect(r.processes[1].finish_time < 13,
               "P2 no sufre starvation gracias al boost");
}

static void testPriorityOrder(testfw::Context& ctx) {
    MlfqEngine engine = buildEngine(1000);
    SimulationResult r = engine.run({makeProcess(1, 0, 2),
                                     makeProcess(2, 0, 3)});

    ctx.expect(r.processes[0].finish_time == 2,
               "P1 termina primero (mayor prioridad)");
}

int main() {
    testfw::Context ctx;

    std::printf("[TEST] metricsComputation\n");
    testMetricsComputation(ctx);

    std::printf("[TEST] metricsSummary\n");
    testMetricsSummary(ctx);

    std::printf("[TEST] demotionOnFullQuantum\n");
    testDemotionOnFullQuantum(ctx);

    std::printf("[TEST] noDemotionWhenFinishingEarly\n");
    testNoDemotionWhenFinishingEarly(ctx);

    std::printf("[TEST] priorityBoostPreventsStarvation\n");
    testPriorityBoostPreventsStarvation(ctx);

    std::printf("[TEST] priorityOrder\n");
    testPriorityOrder(ctx);

    return ctx.summary();
}