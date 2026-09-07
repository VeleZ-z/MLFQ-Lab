#ifndef MLFQ_QUEUE_H
#define MLFQ_QUEUE_H

#include <cstddef>
#include <vector>

namespace domain {

// Abstraccion de cola FIFO usada por el scheduler.
// Elegimos encapsular la politica de desalojo dentro de una clase en vez de
// operar directamente sobre un std::vector para poder variar la
// implementacion (DIP: el scheduler depende de esta interfaz, no de una
// estructura concreta) y para respetar el encapsulamiento del estado interno.
class IQueue {
public:
    virtual ~IQueue() = default;
    virtual void push(int pid) = 0;
    virtual int  front() const = 0;
    virtual void pop() = 0;
    virtual bool empty() const = 0;
    virtual std::size_t size() const = 0;
};

// Implementacion FIFO con Round Robin soportado por el llamador al
// re-encolar el proceso (push al final tras agotar su quantum sin terminar
// ni ser demovido).
class FifoQueue final : public IQueue {
public:
    explicit FifoQueue(int quantum) : quantum_(quantum) {}

    int quantum() const { return quantum_; }

    void push(int pid) override { pids_.push_back(pid); }
    int  front() const override { return pids_.front(); }
    void pop() override { pids_.erase(pids_.begin()); }
    bool empty() const override { return pids_.empty(); }
    std::size_t size() const override { return pids_.size(); }

private:
    std::vector<int> pids_;
    int quantum_;
};

} // namespace domain

#endif