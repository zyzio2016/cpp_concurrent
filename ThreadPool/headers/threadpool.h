#ifndef __THREAD_POOL_H_GUARD__
#define __THREAD_POOL_H_GUARD__

#include <chrono>
#include <future>
#include <memory>
#include <functional>
#include "executor.h"

namespace zyzio {
    namespace concurrent {

        //internal implementation
        class internal_executor_service {
        public:
            struct task_wrapper {
                runnable* task = nullptr;
                bool deleteAfter = false;
            };
            
            virtual ~internal_executor_service() {}
            virtual void addToQueue(std::function<void()> f) = 0;
            virtual void shutdown() = 0;
            virtual bool isShutdown() = 0;
        };

        //abstract executor service
        class executor_service : public executor {
        protected:
            ::std::unique_ptr< internal_executor_service> impl;
        public:
            executor_service(internal_executor_service* internal) : impl(internal) {}

            virtual ~executor_service() {}

            void execute(runnable& command);
            void execute(std::function<void()> f);

            ////Submits a value - returning task for execution and returns a Future representing the pending results of the task.
            //template<class T, class callable>
            //std::future< T> submit(callable task);

            ////Submits a Runnable task for execution and returns a Future representing that task.
            //template<class runnable>
            //std::future< void> submit(runnable task);
            template<class F, class... Args>
            ////Submits a task for execution and returns a Future representing that task.
            //template<class runnable>
            auto submit(F f, Args&&... args)
                ->std::future<typename std::result_of<F(Args...)>::type>;

            ////Submits a Runnable task for execution and returns a Future representing that task.
            //template<class T, class runnable>
            //std::future< T> submitRunnable(runnable task, T result);

            //Blocks until all tasks have completed execution after a shutdown request, or the timeout occurs, 
            //or the current thread is interrupted, whichever happens first.
            //template<class Rep, class Period>
            //bool awaitTermination(const std::chrono::duration< Rep, Period>& timeout_duration);

            ////Executes the given tasks, returning a list of Futures holding their status and results when all complete.
            //template <class InputIterator, class T>
            //std::vector< std::future<T>> invokeAll(InputIterator task_begin, InputIterator task_end);

            ////Executes the given tasks, returning a list of Futures holding their status and results when all complete or the timeout expires, 
            ////whichever happens first.
            //template <class InputIterator, class T, class Rep, class Period>
            //std::vector< std::future< T> > invokeAll(InputIterator task_begin, InputIterator task_end, const std::chrono::duration< Rep, Period>& timeout_duration);

            ////Returns true if this executor has been shut down.
            bool isShutdown();

            ////Returns true if all tasks have completed following shut down.
            //bool isTerminated();

            ////Initiates an orderly shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
            void shutdown();
        };

        class RunnableWrapper : public runnable {
        public:
            typedef std::function< void()> caller_type;
        private:
            caller_type fnct;
        public:
            RunnableWrapper(caller_type caller) {
                fnct = move(caller);
            }

            void run() {
                fnct();
            }
        };

        template<class F, class... Args>
        auto executor_service::submit(F f, Args&&... args)
            ->std::future<typename std::result_of<F(Args...)>::type> {

            using return_type = typename std::result_of<F(Args...)>::type;
            auto task = std::make_shared< std::packaged_task< return_type()> >(std::bind(std::forward< F>(f), std::forward< Args>(args)...));
            impl->addToQueue([task]() { (*task)(); });
            return task->get_future();
        }
    }
}
#endif


