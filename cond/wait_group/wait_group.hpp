#pragma once

#include <cstdlib>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/atomic.hpp>

class WaitGroup {
 public:
  // += count
  void Add(size_t cnt) {
    cnt_.fetch_add(cnt);
  }

  // =- 1
  void Done() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
    cnt_.fetch_sub(1);
    if (cnt_.load() == 0) {
      cv_.notify_all();
    }
  }

  // == 0
  // One-shot
  void Wait() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
    while (cnt_.load() != 0) {
      cv_.wait(lock);
    }

  }

 private:
  twist::ed::stdlike::condition_variable cv_;
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::atomic<uint32_t> cnt_{0};

};
