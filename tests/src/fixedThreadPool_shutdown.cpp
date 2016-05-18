#include <iostream>
#include "executors.h"
#include "TestThreadRunner.h"

using namespace std;
using namespace zyzio::concurrent;

class ShutdownRunner : public TestThreadRunner {
public:
    void run() {
        this_thread::sleep_for(chrono::seconds(5));
        TestThreadRunner::run();
    }
};

bool fixedThreadPool_shutdown() {
    try {
        mutex mtx;
        ShutdownRunner runner;
        unique_ptr<executor_service> threadPool(executors::newFixedThreadPool(1));
        executor_service* ppool = threadPool.get();
        threadPool->execute(runner);
        threadPool->execute(runner);
        {
            ShutdownRunner* prunner = &runner;
            unique_lock<mutex> lck(runner.mtx);
            runner.cv.wait_for(lck, chrono::seconds(10));
            if (runner.count != 1)
                throw exception("Wait for fixedThreadPool.shutdown failed");
        }
        threadPool->shutdown();
        {
            unique_lock<mutex> lck(runner.mtx);
            bool stat = runner.cv.wait_for(lck, chrono::seconds(30), [ppool]() { return ppool->isShutdown(); });
            if (!stat)
                throw exception("Wait for fixedThreadPool.shutdown failed - still running");
            cout << "fixedThreadPool.shutdown succeded" << endl;
        }
        return true;
    } catch (exception& e) {
        cerr << "fixedThreadPool.shutdown failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "fixedThreadPool.shutdown failed. Reason unknown." << endl;
    }
    return true;
}
