#include <queue>
#include <vector>
#include <thread>
#include <atomic>
#include "../headers/threadpool.h"

namespace zyzio {
    namespace concurrent {

        class fixed_thread_pool : public internal_executor_service {
        public:
            typedef std::queue< std::function< void()> > tasks_queue;
            typedef std::vector< std::thread> thread_list;

        protected:
            // the thread list
            thread_list workers;
            // the task queue - FILO
            tasks_queue tasks;
            // synchronization for queue task
            std::mutex queue_mutex;
            volatile bool stop;
            std::condition_variable condition;
            std::atomic_size_t workingThreads;

        public:
            fixed_thread_pool(size_t nThreads);
            ~fixed_thread_pool();
            void addToQueue(std::function<void()>);
            void shutdown();
            bool isShutdown();
            bool isTerminated() {
                return workingThreads == 0;
            }
        };
    }
}