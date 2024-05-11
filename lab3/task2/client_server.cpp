#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <future>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <iomanip>


template<typename T>
T sin_task(T x, T) {
    return std::sin(x);
}

template<typename T>
T sqrt_task(T x, T) {
    return std::sqrt(x);
}

template<typename T>
T power_task(T base, T exponent) {
    return std::pow(base, exponent);
}

template<typename T>
class Server {
public:
    Server() : running(true) {}

    void start() {
        server_thread = std::thread(&Server::run, this);
    }

    void stop() {
        running = false;
        server_thread.join();
    }

    size_t add_task(std::packaged_task<T()> task) {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        std::future<T> result = task.get_future();
        size_t id = ++ind;
        results[id] = result.share();  
        tasks.push(std::move(task));
        ids.push(id);
        return id;
    }

    double request_result(size_t id_res) {
        std::shared_future<T> future;
        {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            future = results.find(id_res)->second;
        }
        return future.get();
    }

private:
    std::queue<std::packaged_task<T()>> tasks;
    std::queue<size_t> ids;
    std::unordered_map<size_t, std::shared_future<T>> results;
    std::mutex tasks_mutex;
    std::mutex results_mutex;
    std::thread server_thread;
    size_t ind = 0;
    bool running;

    void run() {
        size_t id;
        std::packaged_task<T()> task;
        while (running) {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            if (!tasks.empty()) {
                id = ids.front();
                task = std::move(tasks.front());
                tasks.pop();
                ids.pop();
                lock.unlock();
                task();
            } else {
                lock.unlock();
                if (!running)
                    break;
            }
        }
    }
};


template<typename T>
class Client {
public:
    Client(Server<T>& server, size_t num_tasks, std::function<T(T, T)> task, std::string path) 
    : server(server), num_tasks(num_tasks), task(task), path(path){}

    void run() {
        std::ofstream file(path);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(1.0, 5.0);

        
        
        for (size_t i = 0; i < num_tasks; ++i) {
            T arg1 = dis(gen);
            T arg2 = dis(gen);
            std::packaged_task<T()> task_composed(std::bind(task, arg1, arg2));
            size_t id = server.add_task(std::move(task_composed));
            T ans = server.request_result(id);
            file << std::fixed << std::setprecision(10);
            file << arg1 << ' ' << arg2  << ' ' << ans << std::endl;
        }
        file.close();
    }

private:
    Server<T>& server;
    size_t num_tasks;
    std::function<T(T,T)> task;
    std::string path;
};



int main() {
    constexpr size_t num_tasks_per_client = 10000;
    Server<double> server;
    server.start();
   

    Client<double> client1(server, num_tasks_per_client, sin_task<double>, "sins.txt");
    Client<double> client2(server, num_tasks_per_client, sqrt_task<double>, "sqrt.txt");
    Client<double> client3(server, num_tasks_per_client, power_task<double>, "pow.txt");

    std::thread thread1(&Client<double>::run, &client1);
    std::thread thread2(&Client<double>::run, &client2);
    std::thread thread3(&Client<double>::run, &client3);

    thread1.join();
    thread2.join();
    thread3.join();

    server.stop();

    return 0;
}