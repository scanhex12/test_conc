#include <exe/fibers/sched/go.hpp>

namespace exe::fibers {

void Go(Scheduler& scheduler, Routine routine) {
  exe::coro::Coroutine coroutine(std::move(routine));
  scheduler.Submit([&]() mutable {
    std::cout << "START\n";
    coroutine.Resume();
    std::cout << "END\n";
  });
}

void Go(Routine routine) {
  exe::coro::Coroutine coroutine(std::move(routine));
  coroutine.Resume();
}

}  // namespace exe::fibers
