#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/atomic.hpp>

#include <twist/ed/stdlike/condition_variable.hpp>

#include <optional>

#include "tagged_semaphore.h"
#include <twist/ed/stdlike/mutex.hpp>
#include <deque>
#include <queue>
#include <iostream>
#include <mutex>
#include <memory>

namespace tp {

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue

template <typename T>
class UnboundedBlockingQueue {
 public:

  bool Put(T value) {
    if (closed_.load() == 1) {
      return false;
    }
    mutex_.lock();
    elems_.push(std::move(value));
    mutex_.unlock();
    cv_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
    while (elems_.empty() && closed_.load() == 0) {
      cv_.wait(lock);
    }
    if (closed_.load() == 1 && elems_.empty()) {
      return std::nullopt;
    }
    std::optional<T> answer = std::nullopt;
    if (!elems_.empty()) {
      answer = std::move(elems_.front());
      elems_.pop();
    }

    return std::move(answer);
  }

  void Close() {
    mutex_.lock();
    closed_.store(1);
    mutex_.unlock();
    cv_.notify_all();
  }

  ~UnboundedBlockingQueue() {
    //cv_idle_ = nullptr;
    //task_ended_ = nullptr;
    Close();
  }

 private:
  // Buffer
  std::queue<T> elems_;

  twist::ed::stdlike::mutex mutex_;

  twist::ed::stdlike::atomic<uint32_t> closed_{0};
  twist::ed::stdlike::condition_variable cv_;
};

}  // namespace tp
