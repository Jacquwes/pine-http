#include <functional>
#include <mutex>
#include <type_traits>
#include <vector>
#include "thread_pool.h"

namespace pine
{
  thread_pool::thread_pool(size_t num_threads)
  {
    this->start_pool(num_threads);
  }

  thread_pool::~thread_pool()
  {
    this->stop_pool();
    this->condition.notify_all();
    for (auto& thread : this->threads)
      thread.join();
  }

  void thread_pool::start_pool(size_t num_threads)
  {
    for (size_t i = 0; i < num_threads; i++)
    {
      this->threads.emplace_back(
        [this]
        {
          while (true)
          {
            std::function<void()> task;
            {
              std::unique_lock lock(this->queue_mutex);
              this->condition.wait(lock, [this]
                                   {
                                     return this->stop || !this->tasks.empty();
                                   });
              if (this->stop && this->tasks.empty())
                return;
              task = std::move(this->tasks.front());
              this->tasks.pop();
            }
            task();
          }
        });
    }
  }

  void thread_pool::stop_pool()
  {
    std::unique_lock lock(this->queue_mutex);
    this->stop = true;
  }
}
