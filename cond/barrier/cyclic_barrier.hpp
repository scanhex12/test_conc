#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdlib>
#include <mutex>

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t participants):thr_(participants), participants_(participants) {
  }

  void ArriveAndWait() {
      std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
      thr_--;
      size_t cur_local = cur_;
      if (thr_ == 0) {
        cur_++;
        thr_ = participants_;
        cv_.notify_all();
      } else {
        cv_.wait(lock, [cur_local, this] { return cur_ != cur_local; });
      }
  }

 private:
  size_t thr_;
  size_t cur_ = 0;
  size_t participants_;
  twist::ed::stdlike::condition_variable cv_;
  twist::ed::stdlike::mutex mutex_;
};
