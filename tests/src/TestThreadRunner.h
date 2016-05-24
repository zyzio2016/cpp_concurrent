#ifndef __TEST_THREAD_RUNNER_H_GUARD__
#define __TEST_THREAD_RUNNER_H_GUARD__


#include "executor.h"
#include <mutex>
#include <condition_variable>
#include <atomic>

class TestThreadRunner : public zyzio::concurrent::runnable {
public:
    int count = 0;
    std::mutex mtx;
    std::condition_variable cv;

    void run() {
        std::unique_lock<std::mutex> lck(mtx);
        count++;
        cv.notify_all();
    }
};

template<class T>
class TestThreadCallable : public zyzio::concurrent::callable<T> {
public:
    int count = 0;
    std::mutex mtx;
    std::condition_variable cv;

    T call () {
        std::unique_lock<std::mutex> lck(mtx);
        count++;
        cv.notify_all();
        return count;
    }
};

class TestRunnable : public zyzio::concurrent::runnable {
public:
    static std::atomic_uint counter;
    void run() {}
    ~TestRunnable() {
        counter++;
    }
};

template<class T>
class TestCallable : public zyzio::concurrent::callable<T> {
public:
    static std::atomic_uint counter;
private:
    T value;
public:
    TestCallable(T v) : value(v) {}
    ~TestCallable() {
        counter++;
    }

    T call() {
        return value;
    }
};

#endif