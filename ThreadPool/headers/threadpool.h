#ifndef __THREAD_POOL_H_GUARD__
#define __THREAD_POOL_H_GUARD__

#include <chrono>
#include <future>
#include <memory>
#include <functional>
#include <algorithm>
#include "executor.h"

namespace zyzio {
    namespace concurrent {

        ///internal implementation
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

        ///abstract executor service
        class executor_service : public executor {
        protected:
            ::std::unique_ptr< internal_executor_service> impl;

        public:
            ///create a service, internal is a real thread pool implementation.
            ///The internal implementation is automatically delete in executor_service
            executor_service(internal_executor_service* internal) : impl(internal) {}

            void execute(runnable& command) {
                impl->addToQueue([&command]() {command.run(); });
            }

            ///execute runnable and delete the object
            ///removeAfter - delete runnable after executing or canceling
            void execute(runnable* command, bool removeAfter = false) {
                if (removeAfter) {
                    auto pCommand = std::shared_ptr< runnable>(command);
                    impl->addToQueue([pCommand]() {pCommand->run(); });
                } else {
                    impl->addToQueue([command]() {command->run(); });
                }
            }

            void execute(std::function<void()> f) {
                impl->addToQueue(f);
            }

            ///Submits a runnable task for execution and returns a Future representing that task.
            std::future< void> submitRunnable(runnable& command) {
                return submit([&command]() { command.run(); });
            }

            ///Submits a runnable task for execution and returns a Future representing that task.
            ///removeAfter - delete runnable after executing or canceling
            std::future< void> submitRunnable(runnable* command, bool removeAfter = false) {
                if (removeAfter) {
                    auto pCommand = std::shared_ptr< runnable>(command);
                    return submit([pCommand]() {pCommand->run(); });
                } else {
                    return submit([command]() {command->run(); });
                }
            }

            ///Submits a value - returning task for execution and returns a future representing the pending results of the task.
            template<class T>
            std::future<T> submitCallable(callable<T>& command) {
                return submit([&command]() { return command.call(); });
            }

            ///Submits a value - returning task for execution and returns a future representing the pending results of the task.
            template<class T>
            std::future<T> submitCallable(callable<T>* command, bool removeAfter = false) {
                if (removeAfter) {
                    auto pCommand = std::shared_ptr< callable<T>>(command);
                    return submit([pCommand]() { return pCommand->call(); });
                } else {
                    return submit([command]() { return command->call(); });
                }
            }

            ///Submits a task for execution and returns a Future representing that task.
            template<class F, class... Args>
            auto submit(F f, Args&&... args)
                ->std::future<typename std::result_of<F(Args...)>::type>;

            ///Executes the given tasks, returning a list of futures holding their status and results when all complete.
            ///Executes the given tasks, returning a list of futures holding their status and results when all complete.
            template <class InputIterator>
            std::vector< std::future<void> > invokeAllRunnable(InputIterator task_begin, InputIterator task_end) {
                std::vector< std::future<void> > futures;
                std::for_each(task_begin, task_end, [&futures, this](auto t) { 
                    futures.push_back(submitRunnable(std::move(t))); 
                });
                std::for_each(futures.begin(), futures.end(), [](std::future<void>& f) { f.wait(); });
                return futures;
            }

            template <class InputIterator>
            std::vector< std::future<void> > invokeAllRunnable(InputIterator task_begin, InputIterator task_end, bool removeAfter) {
                std::vector< std::future<void> > futures;
                std::for_each(task_begin, task_end, [&futures, removeAfter, this](auto t) { 
                    futures.push_back(submitRunnable(std::move(t), removeAfter)); 
                });
                std::for_each(futures.begin(), futures.end(), [](std::future<void>& f) { f.wait(); });
                return futures;
            }

            ///Executes the given tasks, returning a list of futures holding their status and results when all complete.
            template <class InputIterator, class T>
            std::vector< std::future<T> > invokeAllCallable(InputIterator task_begin, InputIterator task_end) {
                std::vector< std::future<T> > futures;
                std::for_each(task_begin, task_end, [&futures, this](auto f) { 
                    futures.push_back(submitCallable(std::move(f))); 
                });
                std::for_each(futures.begin(), futures.end(), [](std::future<T>& f) { f.wait(); });
                return futures;
            }

            template <class InputIterator, class T>
            std::vector< std::future<T> > invokeAllCallable(InputIterator task_begin, InputIterator task_end, bool removeAfter) {
                std::vector< std::future<T> > futures;
                std::for_each(task_begin, task_end, [&futures, removeAfter, this](auto f) { 
                    futures.push_back(submitCallable(std::move(f), removeAfter)); 
                });
                std::for_each(futures.begin(), futures.end(), [](std::future<T>& f) { f.wait(); });
                return futures;
            }

            ///Executes the given tasks, returning a list of futures holding their status and results when all complete.
            template <class InputIterator, class F, class T>
            std::vector< std::future<T> > invokeAll(InputIterator task_begin, InputIterator task_end) {
                std::vector< std::future<T> > futures;
                std::for_each(task_begin, task_end, [&futures, this](F& f) { 
                    futures.push_back(submit(f));
                });
                std::for_each(futures.begin(), futures.end(), [](std::future<T>& f) { f.wait(); });
                return futures;
            }

            ///Executes the given tasks, returning a list of Futures holding their status and results when all complete or the timeout expires, 
            ///whichever happens first.
            template <class InputIterator, class Duration>
            std::vector< std::future< void> > invokeAll(InputIterator task_begin, InputIterator task_end, const Duration& timeout_duration);

            //Blocks until all tasks have completed execution after a shutdown request, or the timeout occurs, 
            //or the current thread is interrupted, whichever happens first.
            //template<class Rep, class Period>
            //bool awaitTermination(const std::chrono::duration< Rep, Period>& timeout_duration);

            ///Returns true if this executor has been shut down.
            bool isShutdown() {
                return impl->isShutdown();
            }

            ///Returns true if all tasks have completed following shut down.
            //bool isTerminated();

            ///Initiates an orderly shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
            void shutdown() {
                impl->shutdown();
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

        class wait_runner : public runnable {
        public:
            std::shared_ptr< std::condition_variable> wait;
            std::shared_ptr< std::mutex> waitMutex;
            std::shared_ptr< size_t> waitCounter;
            std::function<void()> funct;

            void run() {
                funct();
                notify();
            }

            void notify() {
                if (wait.get() != nullptr) {
                    std::unique_lock< std::mutex> lck(*waitMutex);
                    --(*waitCounter);
                    wait->notify_one();
                    wait.reset();
                }
            }

            ~wait_runner() {
                notify();
            }
        };

        template <class InputIterator, class Duration>
        std::vector< std::future< void> > executor_service::invokeAll(InputIterator task_begin, InputIterator task_end, 
            const Duration& timeout_duration) {

            std::vector< std::future< void> > futures;
            auto wait4all = std::shared_ptr< std::condition_variable>(new std::condition_variable());
            std::shared_ptr< std::mutex> waitMutex (new std::mutex());
            std::shared_ptr< size_t> waitCounter (new size_t(0));
            size_t taskCounter = 0;
            std::for_each(task_begin, task_end, [=, &futures, &taskCounter](std::function<void()>f) {
                wait_runner* r = new wait_runner();
                r->funct = std::move(f);
                r->wait = wait4all;
                r->waitCounter = waitCounter;
                r->waitMutex = waitMutex;
                taskCounter++;
                futures.push_back(submitRunnable(r, true)); 
            });
            std::unique_lock<std::mutex> lck(*waitMutex);
            wait4all->wait_for(lck, timeout_duration, [taskCounter, counter = waitCounter.get()]{ return taskCounter > *counter; });
            return futures;
        }

    }
}
#endif


