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
  explicit async_result(T value) : value(value) {}

  /// @brief Constructor for async_result with an error code.
  /// @param error The error code associated with the async result.
  explicit async_result(std::error_code error) : error(error) {}
};

/// @brief An awaitable coroutine that returns a value.
/// @tparam T The type of the value to return.
/// @details For more information see: https://en.cppreference.com/w/cpp/language/coroutines
template <typename T>
	requires (!std::is_void_v<T>)
struct async_operation
{
	struct promise_type
	{
		T _value;
		async_operation<T> get_return_object() noexcept { return async_operation<T>(std::coroutine_handle<promise_type>::from_promise(*this)); }
		std::suspend_never initial_suspend() const noexcept { return {}; }
		std::suspend_always final_suspend() const noexcept { return {}; }
		void unhandled_exception() const {}
		void return_value(T value) { _value = value; }
	};

	bool await_ready() const { return false; }
	void await_suspend(std::coroutine_handle<> h) const { h.resume(); }
	auto await_resume() { return _coroutine.promise()._value; }

	std::coroutine_handle<promise_type> _coroutine = nullptr;

	async_operation() = default;
	explicit async_operation(std::coroutine_handle<promise_type> coroutine) : _coroutine(coroutine) {}
	async_operation(async_operation const&) = delete;
	async_operation(async_operation&& other) noexcept
		: _coroutine(other._coroutine)
	{
		other._coroutine = nullptr;
	}
	~async_operation()
	{
		if (_coroutine.address()) _coroutine.destroy();
	}

	async_operation& operator=(async_operation&& other) noexcept
	{
		if (&other != this)
		{
			_coroutine = other._coroutine;
			other._coroutine = nullptr;
		}
	}
};

/// @brief An awaitable coroutine that returns nothing.
struct async_task
{
	struct promise_type
	{
		async_task get_return_object() const noexcept { return {}; }
		std::suspend_never initial_suspend() const noexcept { return {}; }
		std::suspend_always final_suspend() const noexcept { return {}; }
		void unhandled_exception() const {}
		void return_void() const {}
	};

	bool await_ready() const noexcept { return false; }
	void await_suspend(std::coroutine_handle<> h) const noexcept { h.resume(); }
	void await_resume() const noexcept {}
};
