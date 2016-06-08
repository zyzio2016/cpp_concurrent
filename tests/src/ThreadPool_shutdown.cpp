#include <iostream>
#include"TestException.h"
#include "executors.h"
#include "TestThreadRunner.h"

using namespace std;
using namespace zyzio::concurrent;

class ShutdownRunner : public TestThreadRunner {
public:
    void run() {
        this_thread::sleep_for(chrono::seconds(2));
        TestThreadRunner::run();
    }
};

bool ThreadPool_shutdown() {
    try {
        mutex mtx;
        ShutdownRunner runner;
        unique_ptr<executor_service> threadPool(executors::newSingleThreadExecutor());
        threadPool->execute(runner);
        threadPool->execute(runner);
        threadPool->execute(runner);
        {
            unique_lock<mutex> lck(runner.mtx);
            runner.cv.wait_for(lck, chrono::seconds(3));
            if (runner.count != 1)
                throw TestException("Wait for fixedThreadPool.shutdown failed");
        }
        if (threadPool->isTerminated())
            throw TestException("fixedThreadPool.isTerminated failed - already terminated");

        threadPool->shutdown();
        {
            unique_lock<mutex> lck(runner.mtx);
            bool stat = runner.cv.wait_for(lck, chrono::seconds(4), [&threadPool]() { return threadPool->isShutdown(); });
            if (!stat)
                throw TestException("Wait for fixedThreadPool.shutdown failed - still running");
            if (runner.count > 2) 
                throw TestException("Wait for fixedThreadPool.shutdown failed - to many runner were run");
            if (threadPool->isTerminated())
                throw TestException("fixedThreadPool.isTerminated failed - a pool isn't terminated");
            cout << "fixedThreadPool.shutdown succeded" << endl;
        }
        return true;
    } catch (exception& e) {
        cerr << "fixedThreadPool.shutdown failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "fixedThreadPool.shutdown failed. Reason unknown." << endl;
    }
    return false;
}
