#pragma once

#include <stdlike/future.hpp>
#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <mutex>
#include <memory>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise() {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() {
    value_ = std::make_shared<T>();
    cv_ = std::make_shared<twist::ed::stdlike::condition_variable>();
    exception_ = std::make_shared<std::exception_ptr>();
    cnt_ = std::make_shared<twist::ed::stdlike::atomic<uint32_t>>(0);
    mutex_ = std::make_shared<twist::ed::stdlike::mutex>();
    return Future<T>(value_, exception_, cv_,mutex_, cnt_);
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T value) {
    {
      std::lock_guard<twist::ed::stdlike::mutex> lock(*mutex_);
      *value_ = std::move(value);
      cnt_->store(1);
    }
    cv_->notify_all();
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr exp) {
    {
      std::lock_guard<twist::ed::stdlike::mutex> lock(*mutex_);
      *exception_ = exp;
      cnt_->store(1);
    }
    cv_->notify_all();
  }

 private:
  std::shared_ptr<twist::ed::stdlike::condition_variable> cv_;
  std::shared_ptr<std::exception_ptr> exception_;
  std::shared_ptr<T> value_;
  std::shared_ptr<twist::ed::stdlike::mutex> mutex_;

  std::shared_ptr<twist::ed::stdlike::atomic<uint32_t>> cnt_;
};

}  // namespace stdlike
