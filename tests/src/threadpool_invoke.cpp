#include <iostream>
#include <vector>
#include "executors.h"
#include "TestException.h"
#include "TestThreadRunner.h"

using namespace std;
using namespace zyzio::concurrent;

struct ConstTestRunner : public runnable {
    TestThreadRunner* runner;
    void run() {
        runner->run ();
    }
};

bool ThreadPool_invoke() {

    TestThreadRunner runner;
    unique_ptr<executor_service> threadPool(executors::newSingleThreadExecutor());
    try {
        vector<ConstTestRunner> list;
        for (int i = 0; i < 10; i++) {
            list.push_back(ConstTestRunner ());
            list.back().runner = &runner;
        }
        auto v = threadPool->invokeAllRunnable(list.begin(), list.end ());
        if (runner.count != 10)
            throw TestException("ThreadPool_invoke (invokeAllRunnable) failed.");

        cout << "ThreadPool_invoke succeded" << endl;
        return true;
    } catch (exception& e) {
        cerr << "ThreadPool_invoke failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "ThreadPool_invoke failed. Reason unknown." << endl;
    }
    return false;
}
