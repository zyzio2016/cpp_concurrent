#ifndef __THREAD_POOL_H_GUARD__
#define __THREAD_POOL_H_GUARD__

#include <chrono>
#include <future>
#include <memory>
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
            virtual void addToQueue(runnable& command, bool deleteAfter) = 0;
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

            void execute(runnable& command, bool deleteAfter = false);

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


            ////Submits a value - returning task for execution and returns a Future representing the pending results of the task.
            //template<class T, class callable>
            //std::future< T> submit(callable task);

            ////Submits a Runnable task for execution and returns a Future representing that task.
            //template<class runnable>
            //std::future< void> submit(runnable task);

            ////Submits a Runnable task for execution and returns a Future representing that task.
            //template<class T, class runnable>
            //std::future< T> submitRunnable(runnable task, T result);
        };
    }
}
#endif


