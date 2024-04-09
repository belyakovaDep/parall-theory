#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <unordered_map>

std::mutex mut;

template <typename T> class Task {
public:
  Task() = default;
  ~Task() = default;

  virtual void GenerateAndRunTask() = 0;

  T GetTaskResult() { return this->result; }

protected:
  T result;
};

template <typename T> class SinTask : public Task<T> {
public:
  SinTask() : Task<T>(){};
  virtual ~SinTask() = default;

  void GenerateAndRunTask() override {
    T arg = static_cast<T>(std::rand());
    this->result = this->sin_(arg);
  }

private:
  T sin_(T arg) { return std::sin(arg); }
};

template <typename T> class SqrtTask : public Task<T> {
public:
  SqrtTask() : Task<T>(){};
  virtual ~SqrtTask() = default;

  void GenerateAndRunTask() override {
    T arg = static_cast<T>(std::rand());
    this->result = this->sqrt_(arg);
  }

private:
  T sqrt_(T arg) { return std::sqrt(arg); }
};

template <typename T> class PowTask : public Task<T> {
public:
  PowTask() : Task<T>(){};
  virtual ~PowTask() = default;

  void GenerateAndRunTask() override {
    T value = static_cast<T>(std::rand());
    T power = static_cast<T>(std::rand() % MOD);
    this->result = this->pow_(value, power);
  }

private:
  T pow_(T value, T power) { return std::pow(value, power); }

  static constexpr int MOD = 7;
};

template <typename T> class Server {
public:
  Server() : id_(0), should_work(true) {
    this->thread_ = std::thread(&Server<T>::Executor, this);
    this->thread_.detach();
  }

  ~Server() { this->StopServer(); }

  size_t SetTask(Task<T> &task) {
    std::future<T> new_server_task = std::async(std::launch::deferred, [&]() {
      task.GenerateAndRunTask();
      return task.GetTaskResult();
    });

    std::unique_lock lock_res{mut, std::defer_lock};

    lock_res.lock();
    id_++;
    this->tasks_.push({id_, std::move(new_server_task)});
    lock_res.unlock();

    return id_;
  }

  T GetResult(size_t task_id) {
    if (this->results_.find(task_id) != this->results_.end()) {
      auto result = results_[task_id];
      std::cout << "task_thread result:\t" << result << '\n';
      results_.erase(task_id);

      return result;
    }
  }

  void StartServer() {
    this->should_work = true;
  }

  void StopServer() {
    this->should_work = false;
  }

private:
  // Server thread
  std::thread thread_;
  bool should_work;

  std::queue<std::pair<size_t, std::future<T>>> tasks_;
  std::unordered_map<size_t, T> results_;
  size_t id_;

  void Executor() {
    std::unique_lock lock_res{mut, std::defer_lock};
    while (this->should_work) {
      if (!tasks_.empty()) {
        lock_res.lock();
        if (!tasks_.empty()) {
          size_t task_id = tasks_.front().first;
          auto &current_future = tasks_.front().second;
          auto value = current_future.get();

          results_.insert({task_id, value});
          tasks_.pop();
        }
        lock_res.unlock();
      }
    }
  }
};


template <typename T>
void ClientSendTask(Server<T> &server, Task<T> &new_task, int n) {
  for (auto i = 0; i < n; i++) {
    server.SetTask(new_task);
  }
}

int main() {
  Server<double> server;

  SinTask<double> task_1;
  SqrtTask<double> task_2;
  PowTask<double> task_3;

  // Clients
  std::thread thread_client_1([&]() { ClientSendTask<double>(server, task_1, 100); });
  std::thread thread_client_2([&]() { ClientSendTask<double>(server, task_2, 100); });
  std::thread thread_client_3([&]() { ClientSendTask<double>(server, task_3, 100); });

  thread_client_1.detach();
  thread_client_2.detach();
  thread_client_3.detach();

  std::ofstream fout("output.txt");

  for (auto i = 0; i < 300; i++) {
    fout << server.GetResult(i) << std::endl;
  }

  return 0;
}