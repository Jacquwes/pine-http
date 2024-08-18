#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace pine
{
  /// @brief Thread pool for executing tasks.
  /// @details The thread pool is created with a fixed number of threads that
  /// are used to execute tasks asynchronously. The tasks are enqueued and
  /// executed by the threads in the pool.
  class thread_pool
  {
  public:
    /// @brief Construct a thread pool with a fixed number of threads.
    /// @param num_threads The number of threads in the pool.
    explicit thread_pool(size_t num_threads = std::jthread::hardware_concurrency());

    /// @brief Destroy the thread pool.
    ~thread_pool();

    /// @brief Enqueue a task to be executed by the thread pool.
    /// @tparam task_type The type of the task to enqueue.
    /// @param task The task to enqueue.
    template <typename task_type>
    inline void enqueue(task_type&& task)
    {
      std::unique_lock lock(this->queue_mutex);
      this->tasks.emplace(std::forward<task_type>(task));
      this->condition.notify_one();
    }

  private:
    /// @brief Start the thread pool.
    /// @param num_threads The number of threads in the pool.
    void start_pool(size_t num_threads);

    /// @brief Stop the thread pool.
    void stop_pool();

    std::vector<std::jthread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
  };
}
