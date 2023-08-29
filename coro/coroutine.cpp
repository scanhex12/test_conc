#include "coroutine.hpp"

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/assert.hpp>
#include <wheels/core/compiler.hpp>
#include <wheels/core/size_literals.hpp>
#include <iostream>
#include <stack>

std::stack<Coroutine*> Coroutine::last_seen{};
std::stack<std::exception_ptr> Coroutine::exceptions_stack{};

struct DeleteItem {
  ~DeleteItem() {
    Coroutine::exceptions_stack.pop();
  }
};

using namespace wheels::size_literals;

Coroutine::Coroutine(Routine routine)
    : stack(sure::Stack::AllocateBytes(64_KiB)),
      routine_(std::move(routine))
{
  my_context_func_.Setup(Coroutine::stack.MutView(), this);
}

void Coroutine::Run() noexcept {
  try {
    routine_();
  } catch (...) {
    exceptions_stack.push(std::current_exception());
  }
  completed_ = true;
  auto top = last_seen.top();
  last_seen.pop();
  top->my_context_func_.ExitTo(top->my_context_);

}

void Coroutine::Resume() {
  if (!exceptions_stack.empty()) {
    auto x = DeleteItem();
    std::rethrow_exception(exceptions_stack.top());
  }
  if (!completed_) {
    last_seen.push(this);
    my_context_.SwitchTo(my_context_func_);
  }
  if (!exceptions_stack.empty()) {
    auto x = DeleteItem();
    std::rethrow_exception(exceptions_stack.top());
  }
}

void Coroutine::Suspend() {
  auto top = last_seen.top();
  last_seen.pop();

  top->my_context_func_.SwitchTo(top->my_context_);
}

bool Coroutine::IsCompleted() const {
  return completed_;
}

Coroutine::~Coroutine() {
  std::cout << exceptions_stack.size() << '\n';
  while (!exceptions_stack.empty()) {
    exceptions_stack.pop();
  }
}