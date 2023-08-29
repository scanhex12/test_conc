#pragma once

#include "tagged_semaphore.hpp"
#include <twist/ed/stdlike/mutex.hpp>
#include <deque>
#include <queue>
#include <iostream>
#include <mutex>

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(size_t cap)
      : sem_(cap),
        waiting_sem_(cap) {
    for (size_t i = 0; i < cap; ++i) {
      LockWaitSem();
    }
  }

  void Put(T value) {
    auto sem_token = sem_.Acquire();
    std::lock_guard<twist::ed::stdlike::mutex> lock(mutex_);
    tokens_.push(std::move(sem_token));
    elems_.push(std::move(value));
    UnlockWaitSem();
  }

  T Take() {
    auto wait_token = waiting_sem_.Acquire();
    std::lock_guard<twist::ed::stdlike::mutex> lock(mutex_);

    waiting_tokens_.push(std::move(wait_token));
    T answer = std::move(elems_.front());
    elems_.pop();

    UnlockSem();

    return std::move(answer);
  }

  ~BoundedBlockingQueue() {
    while (!waiting_tokens_.empty()) {
      UnlockWaitSem();
    }
    while (!tokens_.empty()) {
      UnlockSem();
    }
  }

 private:
  // Tags
  struct TagSem {
  };
  struct TagWaitingSem {
  };
  struct TagMutex {
  };

  void LockSem() {
    tokens_.push_back(sem_.Acquire());
  }

  void UnlockSem() {
    sem_.Release(std::move(tokens_.front()));
    tokens_.pop();
  }

  void LockWaitSem() {
    waiting_tokens_.push(waiting_sem_.Acquire());
  }

  void UnlockWaitSem() {
    waiting_sem_.Release(std::move(waiting_tokens_.front()));
    waiting_tokens_.pop();
  }

 private:
  // Buffer
  TaggedSemaphore<TagSem> sem_;
  TaggedSemaphore<TagWaitingSem> waiting_sem_;

  std::queue<T> elems_;

  twist::ed::stdlike::mutex mutex_;

  std::queue<typename TaggedSemaphore<TagSem>::Token> tokens_;
  std::queue<typename TaggedSemaphore<TagWaitingSem>::Token> waiting_tokens_;
};
