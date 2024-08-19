#pragma once

#include <coroutine>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include "thread_pool.h"

/// @brief Represents the result of an async operation.
/// @tparam T The type of the value to return.
template <typename T>
struct async_result
{
  /// @brief The value of the async result.
  T value{};

  /// @brief The error code associated with the async result.
  std::error_code error{};

  /// @brief Default constructor for async_result.
  async_result() = default;

  /// @brief Constructor for async_result with a value.
  /// @param value The value of the async result.
  explicit(false) async_result(T value) : value(value) {}

  /// @brief Constructor for async_result with an error code.
  /// @param error The error code associated with the async result.
  explicit(false) async_result(std::error_code error) : error(error) {}

  explicit(false) operator T() const { return value; }
};

/// @brief An awaitable coroutine that returns a value.
/// @tparam T The type of the value to return.
/// @details For more information see: https://en.cppreference.com/w/cpp/language/coroutines
template <typename T>
  requires (!std::is_void_v<T>)
struct async_operation
{
  /// @brief The promise type of the coroutine.
  /// @details The promise type is responsible for managing the coroutine's
  /// lifetime and returning the result.
  struct promise_type
  {
    /// @brief The promise object that holds the result.
    std::shared_ptr<std::promise<async_result<T>>> promise =
      std::make_shared<std::promise<async_result<T>>>();

    /// @brief The shared pointer to the canceled flag.
    std::shared_ptr<std::atomic_bool> cancelled =
      std::make_shared<std::atomic_bool>(false);

    /// @brief Get the return object of the coroutine.
    /// @return The async_operation object.
    async_operation<T> get_return_object() noexcept
    {
      return async_operation<T>(
        std::coroutine_handle<promise_type>::from_promise(*this),
        this->cancelled
      );
    }

    /// @brief The initial suspend point of the coroutine.
    /// @return std::suspend_never
    std::suspend_never initial_suspend() const noexcept { return {}; }

    /// @brief The final suspend point of the coroutine.
    /// @return std::suspend_always
    std::suspend_always final_suspend() const noexcept { return {}; }

    /// @brief Handle unhandled exceptions in the coroutine.
    void unhandled_exception()
    {
      this->promise->set_value(async_result<T>(
        std::make_error_code(std::errc::operation_canceled)
      ));
    }

    /// @brief Set the return value of the coroutine.
    /// @param result The async_result object to set as the return value.
    void return_value(async_result<T> result)
    {
      if (!*cancelled)
        this->promise->set_value(std::move(result));
    }
  };

  /// @brief Check if the awaitable is ready to resume.
  /// @return True if the awaitable is ready, false otherwise.
  bool await_ready() const
  {
    auto future_status = this->get_future().wait_for(std::chrono::seconds(0));
    bool is_ready = future_status == std::future_status::ready;
    return is_ready;
  }

  /// @brief Suspend the coroutine until it is resumed.
  /// @param h The coroutine handle.
  void await_suspend(std::coroutine_handle<> h) const
  {
    static pine::thread_pool& pool = pine::thread_pool::get_instance();
    pool.enqueue([this, h]
                 {
                   if (!*cancelled)
                     h.resume();
                 });
  }

  /// @brief Get the result of the coroutine.
  /// @return The result of the coroutine.
  async_result<T> await_resume() const
  {
    return this->get_future().get();
  }

  std::coroutine_handle<promise_type> _coroutine = nullptr;
  std::shared_ptr<std::atomic_bool> cancelled = nullptr;

  async_operation() = default;

  /// @brief Constructor for async_operation.
  /// @param coroutine The coroutine handle.
  /// @param canceled The shared pointer to the canceled flag.
  explicit async_operation(std::coroutine_handle<promise_type> coroutine,
                           std::shared_ptr<std::atomic_bool> cancelled)
    : _coroutine(coroutine)
    , cancelled(cancelled)
  {
    static pine::thread_pool& pool = pine::thread_pool::get_instance();
    pool.enqueue([this]
                 {
                   if (!*this->cancelled)
                     this->_coroutine.resume();
                   this->_coroutine.destroy();
                 });
  }

  async_operation(async_operation const&) = delete;

  /// @brief Move constructor for async_operation.
  /// @param other The async_operation object to move from.
  async_operation(async_operation&& other) noexcept
    : _coroutine(other._coroutine)
  {
    other._coroutine = nullptr;
  }

  ~async_operation()
  {
    if (this->_coroutine.address()) this->_coroutine.destroy();
  }

  async_operation& operator=(async_operation&& other) noexcept
  {
    if (&other != this)
    {
      this->_coroutine = other._coroutine;
      other._coroutine = nullptr;
    }
  }

  std::future<async_result<T>> get_future() const
  {
    return this->_coroutine.promise().promise->get_future();
  }

  void cancel()
  {
    *cancelled = true;
  }
};

/// @brief An awaitable coroutine that does not return a value.
/// @details For more information see: https://en.cppreference.com/w/cpp/language/coroutines
struct async_task
{
  /// @brief The promise type of the coroutine.
  /// @details The promise type is responsible for managing the coroutine's
  /// lifetime and returning the result.
  struct promise_type
  {
    /// @brief The promise object that holds the error, if any.
    std::shared_ptr<std::promise<std::error_code>> promise =
      std::make_shared<std::promise<std::error_code>>();

    /// @brief The shared pointer to the canceled flag.
    std::shared_ptr<std::atomic_bool> cancelled =
      std::make_shared<std::atomic_bool>(false);

    /// @brief Get the return object of the coroutine.
    /// @return The async_operation object.
    async_task get_return_object() noexcept
    {
      return async_task(
        std::coroutine_handle<promise_type>::from_promise(*this),
        this->cancelled
      );
    }
    /// @brief The initial suspend point of the coroutine.
    /// @return std::suspend_never
    std::suspend_never initial_suspend() const noexcept { return {}; }
    /// @brief The final suspend point of the coroutine.
    /// @return std::suspend_always
    std::suspend_always final_suspend() const noexcept { return {}; }
    /// @brief Handle unhandled exceptions in the coroutine.
    void unhandled_exception()
    {
      this->promise->set_value(
        std::make_error_code(std::errc::operation_canceled)
      );
    }

    /// @brief Return void from the coroutine.
    void return_void()
    {
      if (!*cancelled)
        this->promise->set_value(std::error_code{});
    }
  };

  /// @brief Check if the awaitable is ready to resume.
  /// @return True if the awaitable is ready, false otherwise.
  bool await_ready() const
  {
    auto future_status = this->get_future().wait_for(std::chrono::seconds(0));
    bool is_ready = future_status == std::future_status::ready;
    return is_ready;
  }

  /// @brief Suspend the coroutine until it is resumed.
  /// @param h The coroutine handle.
  void await_suspend(std::coroutine_handle<> h) const
  {
    static pine::thread_pool& pool = pine::thread_pool::get_instance();
    pool.enqueue([this, h]
                 {
                   if (!*cancelled)
                     h.resume();
                 });
  }

  /// @brief Get the result of the coroutine.
  /// @return The result of the coroutine.
  std::error_code await_resume() const
  {
    return this->get_future().get();
  }

  std::coroutine_handle<promise_type> _coroutine = nullptr;
  std::shared_ptr<std::atomic_bool> cancelled = nullptr;

  async_task() = default;

  /// @brief Constructor for async_task.
  /// @param coroutine The coroutine handle.
  /// @param canceled The shared pointer to the canceled flag.
  async_task(std::coroutine_handle<promise_type> coroutine,
             std::shared_ptr<std::atomic_bool> cancelled)
    : _coroutine(coroutine)
    , cancelled(cancelled)
  {
    static pine::thread_pool& pool = pine::thread_pool::get_instance();
    pool.enqueue([this]
                 {
                   if (!*this->cancelled)
                     this->_coroutine.resume();
                   this->_coroutine.destroy();
                 });
  }

  async_task(async_task const&) = delete;

  /// @brief Move constructor for async_task.
  /// @param other The async_task object to move from.
  async_task(async_task&& other) noexcept
    : _coroutine(other._coroutine)
  {
    other._coroutine = nullptr;
  }

  ~async_task()
  {
    if (this->_coroutine.address()) this->_coroutine.destroy();
  }

  async_task& operator=(async_task&& other) noexcept
  {
    if (&other != this)
    {
      this->_coroutine = other._coroutine;
      other._coroutine = nullptr;
    }
  }

  std::future<std::error_code> get_future() const
  {
    return this->_coroutine.promise().promise->get_future();
  }

  void cancel()
  {
    *cancelled = true;
  }
};
