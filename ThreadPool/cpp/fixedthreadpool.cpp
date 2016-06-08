#include <iostream>
#include "fixedthreadpool.h"

using namespace std;

namespace zyzio {
    namespace concurrent {

        fixed_thread_pool::fixed_thread_pool(size_t nThreads) : stop(false), workingThreads(nThreads) {
            for (size_t i = 0; i < nThreads; ++i)
                workers.emplace_back([this] {
                for (;;) {
                    try {
                        function<void()> task;
                        {
                            unique_lock< mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                                break;
                            task = move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    } catch (exception& e) {
                        cerr << e.what() << endl;
                    } catch (...) {
                        cerr << "fixed_thread_pool worker exception" << endl;
                    }
                }
                --this->workingThreads;
            }
            );
        }

        fixed_thread_pool::~fixed_thread_pool() {
            shutdown();
            for (thread &worker : workers)
                worker.join();
        }

        void fixed_thread_pool::addToQueue(function<void()> f) {
            unique_lock<mutex> lock(queue_mutex);
            tasks.emplace(move(f));
            condition.notify_one();
        }

        void fixed_thread_pool::shutdown() {
            {
                unique_lock< mutex> lock(queue_mutex);
                stop = true;
                while (!tasks.empty()) {
                    tasks.pop();
                }
            }
            condition.notify_all();
        }

        bool fixed_thread_pool::isShutdown() {
            unique_lock< mutex> lock(queue_mutex);
            return stop;
        }
    }
}