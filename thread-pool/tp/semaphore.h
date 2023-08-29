#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <mutex>
#include <cstdlib>

class Semaphore {
 public:
  explicit Semaphore(size_t cap) : cap_(cap) {
  }

  void Acquire() {
    std::unique_lock<twist::ed::stdlike::mutex> lock{mutex_};
    cv_.wait(lock, [this] {
      return cap_ > 0;
    });
    cap_--;
  }

  void Release() {
    std::lock_guard<twist::ed::stdlike::mutex> lock{mutex_};
    cap_++;
    cv_.notify_one();
  }

 private:
  int cap_;
  twist::ed::stdlike::condition_variable cv_;
  twist::ed::stdlike::mutex mutex_;
};
