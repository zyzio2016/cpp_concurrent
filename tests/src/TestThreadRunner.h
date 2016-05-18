#ifndef __TEST_THREAD_RUNNER_H_GUARD__
#define __TEST_THREAD_RUNNER_H_GUARD__


#include "executor.h"
#include <mutex>
#include <condition_variable>

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

#endif