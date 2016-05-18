#include <iostream>
#include "executors.h"
#include "TestThreadRunner.h"

using namespace std;
using namespace zyzio::concurrent;

bool fixedThreadPool_execute() {
    try {
        enum { RUNNER_COUNT = 7 };
        TestThreadRunner runner;
        unique_ptr<executor_service> threadPool(executors::newFixedThreadPool(3));
        for (int i = 0; i < RUNNER_COUNT; ++i) {
            threadPool->execute(runner);
        }
        {
            unique_lock<mutex> lck(runner.mtx);
            TestThreadRunner* prunner = &runner;
            bool stat = runner.cv.wait_for(lck, chrono::minutes(1), [prunner] {return prunner->count == RUNNER_COUNT; });
            if (!stat)
                throw exception("Wait for fixedThreadPool.execute failed");
            cout << "fixedThreadPool.execute succeded" << endl;
            return true;
        }
    } catch (exception& e) {
        cerr << "ixedThreadPool.execute failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "ixedThreadPool.execute failed. Reason unknown." << endl;
    }
    return true;
}
