#pragma once

#include <memory>
#include <cassert>
#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/stdlike/atomic.hpp>


namespace stdlike {

template <typename T>
class Future {
  template <typename U>
  friend class Promise;

 public:
  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Movable
  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  // One-shot
  // Wait for result (value or exception)
  T Get() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(*mutex_);
    while (cnt_->load() == 0) {
      cv_->wait(lock);
    }
    if (*exception_ == nullptr) {
      return std::move(*value_);
    } else {
      rethrow_exception(*exception_);
    }
  }

 private:
  Future(std::shared_ptr<T> const& value,
         std::shared_ptr<std::exception_ptr> const& exception,
         std::shared_ptr<twist::ed::stdlike::condition_variable> const& cv,
         std::shared_ptr<twist::ed::stdlike::mutex> const& mutex,
         std::shared_ptr<twist::ed::stdlike::atomic<uint32_t>> const& cnt)
      : value_(value),
        exception_(exception),
        cv_(cv),
        mutex_(mutex),
        cnt_(cnt)
  {
  }

 private:
  std::shared_ptr<T> value_;
  std::shared_ptr<std::exception_ptr> exception_;
  std::shared_ptr<twist::ed::stdlike::condition_variable> cv_;
  std::shared_ptr<twist::ed::stdlike::mutex> mutex_;
  std::shared_ptr<twist::ed::stdlike::atomic<uint32_t>> cnt_;
};

}  // namespace stdlike
