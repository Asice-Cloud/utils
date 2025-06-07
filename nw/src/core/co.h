#include <coroutine>
#include <exception>
#include <iostream>
#include <string>

#define debug debug1()

class debug1 {
public:
  template <typename T> debug1 &operator,(const T &t) {
    std::cout << "  " << t;
    return *this;
  }

  ~debug1() { std::cout << std::endl; }
};

// struct ReAwaiter {
//   bool await_ready() const noexcept { return false; }
//   std::coroutine_handle<>
//   await_suspend(std::coroutine_handle<> coroutine) const noexcept {
//     if (coroutine.done()) {
//       return std::noop_coroutine();
//     } else {
//       return coroutine;
//     }
//     // return;
//   }
//
//   void await_resume() const noexcept {}
// };
//
// struct ReAwaitable {
//   auto operator co_await() const { return ReAwaiter{}; }
// };

struct Pre_Awaiter {

  std::coroutine_handle<> m_previous;

  bool await_ready() const noexcept { return false; }
  std::coroutine_handle<>
  await_suspend(std::coroutine_handle<> coroutine) const noexcept {
    if (m_previous) {
      return m_previous;
    } else {
      return std::noop_coroutine();
    }
  }

  void await_resume() const noexcept {}
};

template <typename T>
struct Promise {

  Promise() = default;
  ~Promise() = default;

  auto initial_suspend() { return std::suspend_always{}; }

  auto final_suspend() noexcept { return Pre_Awaiter{m_previous}; }

  void unhandled_exception() {
    debug, "unhandled exception";
    m_exception = std::current_exception();
  }

  auto yield_value(int value) {
    m_value = value;

    // return Pre_Awaiter{m_previous};
    return std::suspend_always{};
  }

  void return_value(T value) { m_value = value; }

  T result() const{
    [[unlikely]]if (m_exception)
    {
      std::rethrow_exception(m_exception);
    }

      return m_value;
  }

  // auto return_void() { m_value = 0; }

  std::coroutine_handle<Promise> get_return_object() {
    return std::coroutine_handle<Promise>::from_promise(*this);
  }

  std::coroutine_handle<> m_previous{};
  std::exception_ptr m_exception{};
  T m_value{};
};


template <>
struct Promise<void> {

  Promise() = default;
  ~Promise() = default;

  auto initial_suspend() { return std::suspend_always{}; }

  auto final_suspend() noexcept { return Pre_Awaiter{m_previous}; }

  void unhandled_exception() {
    debug, "unhandled exception";
    m_exception = std::current_exception();
  }

  auto yield_value() {

    // return Pre_Awaiter{m_previous};
    m_exception = nullptr;
    return std::suspend_always{};
  }

  // void return_value()
  // {
  //   m_exception = nullptr;
  // }

  void result() {
    [[unlikely]]if (m_exception)
    {
      std::rethrow_exception(m_exception);
    }
  }

  auto return_void() { m_exception = nullptr; }

  std::coroutine_handle<Promise> get_return_object() {
    return std::coroutine_handle<Promise>::from_promise(*this);
  }

  std::coroutine_handle<> m_previous{};
  std::exception_ptr m_exception{};
};


template<typename T>
struct Task {
  using promise_type = Promise<T>;

  Task(std::coroutine_handle<promise_type> coroutine)
      : m_coroutine(coroutine) {}

  std::coroutine_handle<promise_type> m_coroutine;

  Task(Task &&) = delete;
  ~Task() { m_coroutine.destroy(); }

  struct Awaiter {

    bool await_ready() const noexcept { return mm_coroutine.done(); }
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
      mm_coroutine.promise().m_previous = coroutine;
      return mm_coroutine;
    }

    T await_resume() const noexcept {
      return mm_coroutine.promise().result();
    }

    std::coroutine_handle<promise_type> mm_coroutine;
  };

  auto operator co_await() const { return Awaiter{m_coroutine}; }
};

template<typename T>
struct Task_World {
  using promise_type = Promise<T>;

  Task_World(std::coroutine_handle<promise_type> coroutine)
      : m_coroutine(coroutine) {}

  Task_World(Task_World &&) = delete;
  ~Task_World() { m_coroutine.destroy(); }

  struct World_Awaiter {

    bool await_ready() const noexcept { return false; }
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
      mm_coroutine.promise().m_previous = coroutine;
      return mm_coroutine;
    }

    T await_resume() const noexcept {
      return mm_coroutine.promise().result();
    }

    std::coroutine_handle<promise_type> mm_coroutine;
  };

  auto operator co_await() const { return World_Awaiter{m_coroutine}; }
  std::coroutine_handle<promise_type> m_coroutine;
};
