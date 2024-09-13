#pragma once

#include <atomic>
#include <chrono>
#include <coroutine>
#include <future>
#include <memory>
#include <system_error>
#include <type_traits>
#include "expected.h"
#include "thread_pool.h"

/// @brief An awaitable coroutine that returns a value.
/// @tparam T The type of the value to return.
/// @tparam E The type of the error to return.
/// @details For more information see: https://en.cppreference.com/w/cpp/language/coroutines
template <typename T, typename E>
struct async_operation
{
  /// @brief The promise type of the coroutine.
  /// @details The promise type is responsible for managing the coroutine's
  /// lifetime and returning the result.
  struct promise_type
  {
    /// @brief The promise object that holds the result.
    std::shared_ptr<std::promise<std::expected<T, E>>> promise =
      std::make_shared<std::promise<std::expected<T, E>>>();

    /// @brief The shared pointer to the canceled flag.
    std::shared_ptr<std::atomic_bool> cancelled =
      std::make_shared<std::atomic_bool>(false);

    /// @brief Get the return object of the coroutine.
    /// @return The async_operation object.
    async_operation<T, E> get_return_object() noexcept
    {
      return async_operation<T, E>(
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
        std::make_unexpected(
        std::make_error_code(
        std::errc::operation_canceled)));
    }

    /// @brief Set the return value of the coroutine.
    /// @param result The result of the coroutine.
    void return_value(std::expected<T, E> result)
    {
      if (!*cancelled)
        this->promise->set_value(std::move(result));
    }

    void return_value(T result)
    {
      if (!*cancelled)
        this->promise->set_value(std::move(result));
    }

    void return_value(E error = 0)
    {
      if (!*cancelled)
        this->promise->set_value(std::make_unexpected(std::move(error)));
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
  std::expected<T, E> await_resume() const
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

  std::future<std::expected<T, E>> get_future() const
  {
    return this->_coroutine.promise().promise->get_future();
  }

  void cancel()
  {
    *cancelled = true;
  }
};

/// @brief An awaitable coroutine that returns void.
/// @tparam E The type of the error to return.
/// @details This is a specialization of async_operation for void return type.
template <typename E>
struct async_operation<void, E>
{
  /// @brief The promise type of the coroutine.
  struct promise_type
  {
    std::shared_ptr<std::promise<std::expected<void, E>>> promise =
      std::make_shared<std::promise<std::expected<void, E>>>();

    std::shared_ptr<std::atomic_bool> cancelled =
      std::make_shared<std::atomic_bool>(false);

    async_operation<void, E> get_return_object() noexcept
    {
      return async_operation<void, E>(
        std::coroutine_handle<promise_type>::from_promise(*this),
        this->cancelled
      );
    }

    std::suspend_never initial_suspend() const noexcept { return {}; }
    std::suspend_always final_suspend() const noexcept { return {}; }

    void unhandled_exception()
    {
      this->promise->set_value(std::expected<void, E>(
        std::unexpected(std::make_error_code(std::errc::operation_canceled))
      ));
    }

    void return_value(E error = 0)
    {
      if (!*cancelled)
        this->promise->set_value(std::make_unexpected(std::move(error)));
    }
  };

  bool await_ready() const
  {
    auto future_status = this->get_future().wait_for(std::chrono::seconds(0));
    return future_status == std::future_status::ready;
  }

  void await_suspend(std::coroutine_handle<> h) const
  {
    static pine::thread_pool& pool = pine::thread_pool::get_instance();
    pool.enqueue([this, h]
                 {
                   if (!*cancelled)
                     h.resume();
                 });
  }

  std::expected<void, E> await_resume() const
  {
    return this->get_future().get();
  }

  std::coroutine_handle<promise_type> _coroutine = nullptr;
  std::shared_ptr<std::atomic_bool> cancelled = nullptr;

  async_operation() = default;

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

  async_operation(async_operation&& other) noexcept
    : _coroutine(other._coroutine)
    , cancelled(std::move(other.cancelled))
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
      this->cancelled = std::move(other.cancelled);
      other._coroutine = nullptr;
    }
    return *this;
  }

  std::future<std::expected<void, E>> get_future() const
  {
    return this->_coroutine.promise().promise->get_future();
  }

  void cancel()
  {
    *cancelled = true;
  }
};
