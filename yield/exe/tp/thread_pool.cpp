#include <exe/tp/thread_pool.hpp>

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/panic.hpp>

namespace exe::tp {

ThreadPool* ThreadPool::current_pool = nullptr;
//std::vector<decltype(twist::ed::stdlike::this_thread::get_id())> ThreadPool::worker_thread_ids = {};

ThreadPool::ThreadPool(size_t threads)
    : cnt_threads_(threads) {
}

void ThreadPool::Start() {
  for (size_t i = 0; i < cnt_threads_; ++i) {
    threads_.emplace_back([this]() {
      mutex_curpool_.lock();
      ThreadPool::current_pool = this;
      mutex_curpool_.unlock();
      //worker_thread_ids.emplace_back(twist::ed::stdlike::this_thread::get_id());

      while (ended_.load() == 0) {
        std::optional<Task> task = tasks_.Take();
        if (!task) {
          continue;
        }
        (*task)();
        std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
        tasks_ended_.fetch_sub(1);
        if (tasks_ended_.load() == 0) {
          cv_idle_.notify_all();
        }
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  if (ended_.load() == 0) {
    ThreadPool::Stop();
  }
}

void ThreadPool::Submit(Task task) {
  std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
  tasks_ended_.fetch_add(1);
  tasks_.Put(std::move(task));
}

ThreadPool* ThreadPool::Current() {
  return ThreadPool::current_pool;
  /*
  auto this_thread_id = twist::ed::stdlike::this_thread::get_id();

  for (const auto& worker_thread_id : worker_thread_ids) {
    if (worker_thread_id == this_thread_id) {
      return ThreadPool::current_pool;
    }
  }

  return nullptr;*/
}

void ThreadPool::WaitIdle() {
  std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

  while (tasks_ended_.load() != 0) {
    cv_idle_.wait(lock);
  }
}

void ThreadPool::Stop() {
  ended_.store(1);
  tasks_.Close();
  for (size_t i = 0; i < cnt_threads_; ++i) {
    threads_[i].join();
  }
}

}  // namespace exe::tp
