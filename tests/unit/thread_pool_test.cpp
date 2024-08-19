#include <chrono>
#include <functional>
#include <thread>
#include <vector>
#include "gtest/gtest.h"
#include "thread_pool.h"

TEST(ThreadPoolTests, EnqueueTask)
{
  pine::thread_pool& pool = pine::thread_pool::get_instance();

  int result = 0;
  auto task = [&result]() { result = 42; };

  pool.enqueue(task);

  // Wait for the task to be executed
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(result, 42);
}

TEST(ThreadPoolTests, EnqueueMultipleTasks)
{
  pine::thread_pool& pool = pine::thread_pool::get_instance();

  std::vector<int> results(5, 0);
  std::vector<std::function<void()>> tasks;

  for (int i = 0; i < 5; ++i)
  {
    tasks.emplace_back([i, &results]() { results[i] = i * 10; });
    pool.enqueue(tasks.back());
  }

  // Wait for the tasks to be executed
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  for (int i = 0; i < 5; ++i)
  {
    EXPECT_EQ(results[i], i * 10);
  }
}

TEST(ThreadPoolTests, EnqueueLambdaTask)
{
  pine::thread_pool& pool = pine::thread_pool::get_instance();

  int result = 0;

  pool.enqueue([&result]() { result = 100; });

  // Wait for the task to be executed
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(result, 100);
}

TEST(ThreadPoolTests, EnqueueFunctionObjectTask)
{
  pine::thread_pool& pool = pine::thread_pool::get_instance();

  struct Task
  {
    int& result;

    Task(int& r) : result(r) {}

    void operator()()
    {
      result = 200;
    }
  };

  int result = 0;
  Task task(result);

  pool.enqueue(task);

  // Wait for the task to be executed
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(result, 200);
}
