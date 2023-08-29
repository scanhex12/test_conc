#include "philosopher.hpp"

#include <twist/test/inject_fault.hpp>
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

namespace dining {

Philosopher::Philosopher(Table& table, size_t seat)
    : table_(table),
      seat_(seat),
      left_fork_(table_.LeftFork(seat)),
      right_fork_(table_.RightFork(seat)) {
}

void Philosopher::Eat() {
  AcquireForks();
  EatWithForks();
  ReleaseForks();
}

// Acquire left_fork_ and right_fork_
void Philosopher::AcquireForks() {
  while (true) {
    if (left_fork_.try_lock()) {
      if (right_fork_.try_lock()) {
        break;
      } else {
        left_fork_.unlock();
      }
    }
  }
}

void Philosopher::EatWithForks() {
  table_.AccessPlate(seat_);
  // Try to provoke data race
  table_.AccessPlate(table_.ToRight(seat_));
  ++meals_;
}

// Release left_fork_ and right_fork_
void Philosopher::ReleaseForks() {
  {
    left_fork_.unlock();
    right_fork_.unlock();
  }
}

void Philosopher::Think() {
  // Random pause or context switch
  twist::test::InjectFault();
}

}  // namespace dining
