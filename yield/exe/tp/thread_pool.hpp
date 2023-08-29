// Fixed-size pool of worker threads

#pragma once

#include <exe/tp/task.hpp>

#include <twist/ed/stdlike/thread.hpp>
#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/stdlike/mutex.hpp>

#include <memory>
#include <vector>
#include "queue.h"

namespace exe::tp {

// Fixed-size pool of worker threads

class ThreadPool {
 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  // Launches worker threads
  void Start();

  // Schedules task for execution in one of the worker threads
  void Submit(Task);

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

  // Waits until outstanding work count reaches zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  void Stop();

  static ThreadPool* current_pool;
  //static std::vector<decltype(twist::ed::stdlike::this_thread::get_id())> worker_thread_ids;
 private:
  // Worker threads, task queue, etc
  size_t cnt_threads_;
  UnboundedBlockingQueue<Task> tasks_;
  std::vector<twist::ed::stdlike::thread> threads_;

  twist::ed::stdlike::atomic<uint32_t> ended_{0};
  twist::ed::stdlike::mutex mutex_;

  twist::ed::stdlike::condition_variable cv_idle_;
  twist::ed::stdlike::atomic<uint32_t> tasks_ended_{0};
  twist::ed::stdlike::mutex mutex_curpool_;

};

}  // namespace exe::tp
