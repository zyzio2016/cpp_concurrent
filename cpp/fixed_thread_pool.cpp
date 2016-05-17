#include <atomic>
#include <thread>
#include <deque>
#include <iostream>
#include "../headers/ThreadPool.h"

using namespace std;

namespace zyzio {
    namespace thread_pool {

        class fixed_thread_pool : public internal_executor_service {
        public:
            typedef deque< task_wrapper> tasks_queue;
            typedef vector< thread> thread_list;
        protected:
            // the thread list
            thread_list workers;
            // the task queue - FILO
            tasks_queue tasks;
            // synchronization for queue task
            mutex queue_mutex;
            volatile bool stop;
            condition_variable condition;
        public:

            fixed_thread_pool(size_t nThreads) : stop (false) {
                for (size_t i = 0; i < nThreads; ++i)
                    workers.emplace_back([this] {
                    for (;;) {
                        try {
                            function< void()> task;
                            {
                                unique_lock< mutex> lock(this->queue_mutex);
                                this->condition.wait(lock,
                                    [this] { return this->stop || !this->tasks.empty(); });
                                if (this->stop && this->tasks.empty())
                                    return;
                                task = move(this->tasks.front().task);
                                this->tasks.pop_front();
                            }
                            task();
                        } catch (exception& e) {
                            cerr << e.what() << endl;
                        } catch (...) {
                            cerr << "fixed_thread_pool worker exception" << endl;
                        }
                    }
                }
                );
            }

            void addToQueue(function<void()>& task) {
                unique_lock<mutex> lock(queue_mutex);
                task_wrapper wrapper;
                wrapper.task = task;
                tasks.emplace_back(move(wrapper));
            }

            ~fixed_thread_pool() {
                shutdown();
                for (thread &worker : workers)
                    worker.join();
            }

            task_list shutdown() {
                task_list result;
                {
                    unique_lock< mutex> lock(queue_mutex);
                    stop = true;
                    while (!tasks.empty ()) {
                        result.emplace_back(task_wrapper());
                        result.back ().task = move(tasks.front().task);
                        tasks.pop_back();
                    }
                }
                condition.notify_all();
                return result;
            }
        };
    }
}