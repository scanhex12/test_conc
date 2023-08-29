#include <wheels/test/framework.hpp>
#include <atomic>
#include <tf/sched/spawn.hpp>
#include <tf/sched/yield.hpp>
#include <tf/sync/wait_group.hpp>

using tf::WaitGroup;
using tf::Yield;

void LiveLock() {
  static const size_t kIterations = 100;

  size_t cs_count = 0;

  // TrickyLock state
  size_t thread_count = 0;
  bool should_yield = false;
  bool lock_acquired = false;

  auto contender = [&] {
    for (size_t i = 0; i < kIterations; ++i) {
      // TrickyLock::Lock
      while (!lock_acquired) {
        should_yield = true;
        if (thread_count++ == 0) {
          lock_acquired = true;
          should_yield = false;
        } else {
          thread_count--;
          while (should_yield) {
            Yield();
          }
        }
      }
      // Spinlock acquired
      Yield();
      {
        // Critical section
        ++cs_count;
        ASSERT_TRUE_M(cs_count < 3, "Too many critical sections");
        // End of critical section
      }
      // TrickyLock::Unlock
      lock_acquired = false;
      thread_count--;
      // Spinlock released
      Yield();
    }
  };

  // Spawn two fibers
  WaitGroup wg;
  wg.Spawn(contender).Spawn(contender).Wait();
}
