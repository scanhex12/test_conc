#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>
#include <twist/ed/wait/futex.hpp>
#include <twist/ed/stdlike/atomic.hpp>

#include <cstdlib>

namespace stdlike {

int cmpxchg(twist::ed::stdlike::atomic<uint32_t>& atom, unsigned int expected, int desired) {
  atom.compare_exchange_strong(expected, desired);
  return expected;
}

class Mutex {
 public:
  void Lock() {
    int c = cmpxchg(lock_, 0, 1);
    if (c != 0) {
      while ((c = cmpxchg(lock_, 0, 2)) != 0) {
        if (c == 2 || cmpxchg(lock_, 1, 2) != 0) {
          twist::ed::futex::Wait(lock_, 2);
        }
      }
    }
  }

  void Unlock() {
    if (lock_.exchange(0) == 2) {
      auto wake_key = twist::ed::futex::PrepareWake(lock_);
      twist::ed::futex::WakeOne(wake_key);
    }
  }

  void lock() {
    Lock();
  }

  void unlock() {
    Unlock();
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> lock_ = {0};
};

}  // namespace stdlike
