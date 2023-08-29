#pragma once

#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <function2/function2.hpp>

#include <exception>
#include <stack>

#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <function2/function2.hpp>

#include <exception>

namespace exe::coro {

// Simple stackful coroutine

class Coroutine : public sure::ITrampoline {
 public:
  using Routine = fu2::unique_function<void()>;

  explicit Coroutine(Routine routine);

  void Resume();

  // Suspend running coroutine
  static void Suspend();

  bool IsCompleted() const;
  sure::Stack stack;
  ~Coroutine();

  static thread_local std::stack<Coroutine*> last_seen;
  static thread_local std::stack<std::exception_ptr> exceptions_stack;

 private:
  sure::ExecutionContext my_context_;
  sure::ExecutionContext my_context_func_;

  Routine routine_;
  bool completed_ = false;
  void Run() noexcept override;
};

}  // namespace exe::coro
