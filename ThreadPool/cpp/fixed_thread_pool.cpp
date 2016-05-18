#include <iostream>
#include "fixedthreadpool.h"

using namespace std;

namespace zyzio {
    namespace concurrent {

        class DeleteWrapper {
        public:
            static void deleteRunner(internal_executor_service::task_wrapper& wrapper) {
                if (wrapper.deleteAfter && wrapper.task != nullptr) {
                    delete wrapper.task;
                    wrapper.task = nullptr;
                }
            }
            void operator() (internal_executor_service::task_wrapper* wrapper) {
                deleteRunner(*wrapper);
                delete wrapper;
            }
        };

        fixed_thread_pool::fixed_thread_pool(size_t nThreads) : stop(false) {
            for (size_t i = 0; i < nThreads; ++i)
                workers.emplace_back([this] {
                for (;;) {
                    try {
                        unique_ptr< task_wrapper, DeleteWrapper> wrapper(new task_wrapper());
                        {
                            unique_lock< mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                                return;
                            *wrapper.get() = move(this->tasks.front());
                            this->tasks.pop();
                        }
                        wrapper->task->run();
                    } catch (exception& e) {
                        cerr << e.what() << endl;
                    } catch (...) {
                        cerr << "fixed_thread_pool worker exception" << endl;
                    }
                }
            }
            );
        }

        fixed_thread_pool::~fixed_thread_pool() {
            shutdown();
            for (thread &worker : workers)
                worker.join();
        }

        void fixed_thread_pool::addToQueue(runnable& command, bool deleteAfter) {
            unique_lock<mutex> lock(queue_mutex);
            tasks.push(task_wrapper());
            tasks_queue::reference task = tasks.back();
            task.task = &command;
            task.deleteAfter = deleteAfter;
            condition.notify_one();
        }

        void fixed_thread_pool::shutdown() {
            {
                unique_lock< mutex> lock(queue_mutex);
                stop = true;
                while (!tasks.empty()) {
                    DeleteWrapper::deleteRunner(tasks.front());
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