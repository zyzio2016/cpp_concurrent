#include <iostream>
#include "executors.h"
#include "TestThreadRunner.h"

using namespace std;
using namespace zyzio::concurrent;

class DestructorCounter : public  zyzio::concurrent::runnable {
public:
    static std::atomic_uint counter;
    ~DestructorCounter() {
        counter++;
    }
    void run() {}
};

atomic_uint DestructorCounter::counter;

bool fixedThreadPool_execute() {
    try {
        enum { RUNNER_COUNT = 7, DESTRUCTOR_COUNT = 6 };
        TestThreadRunner runner;
        unique_ptr<executor_service> threadPool(executors::newFixedThreadPool(3));
        for (int i = 0; i < RUNNER_COUNT; ++i) {
            threadPool->execute(runner);
        }
        unique_lock<mutex> lck(runner.mtx);
        bool stat = runner.cv.wait_for(lck, chrono::minutes(1), [&runner] {return runner.count == RUNNER_COUNT; });
        if (!stat)
            throw exception("Wait for fixedThreadPool.execute failed");

        for (int i = 0; i < DESTRUCTOR_COUNT; i++) {
            threadPool->execute(new DestructorCounter(), true);
        }
        DestructorCounter dc;
        threadPool->execute(&dc);
        this_thread::sleep_for(chrono::milliseconds(500));
        if (DestructorCounter::counter != DESTRUCTOR_COUNT) {
            throw new exception("fixedThreadPool.execute: wrong number of DesctructorCounter were freed");
        }

        cout << "fixedThreadPool.execute succeded" << endl;
        return true;
    } catch (exception& e) {
        cerr << "fixedThreadPool.execute failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "fixedThreadPool.execute failed. Reason unknown." << endl;
    }
    return false;
}

bool fixedThreadPool_execute_lambda() {
    try {
        enum { RUNNER_COUNT = 7 };
        int counter = 0;
        mutex mtx;
        condition_variable cv;
        unique_ptr<executor_service> threadPool(executors::newFixedThreadPool(3));
        for (int i = 0; i < RUNNER_COUNT; ++i) {
            threadPool->execute([&counter, &mtx, &cv]() { 
                unique_lock<mutex> lck(mtx);
                counter++;
                cv.notify_one();
            });
        }
        unique_lock<mutex> lck(mtx);
        bool stat = cv.wait_for(lck, chrono::minutes(1), [&counter] {return counter == RUNNER_COUNT; });
        if (!stat)
            throw exception("Wait for fixedThreadPool.execute_lambda failed");
        cout << "fixedThreadPool.execute_lambda succeded" << endl;
        return true;
    } catch (exception& e) {
        cerr << "fixedThreadPool.execute_lambda failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "fixedThreadPool.execute_lambda failed. Reason unknown." << endl;
    }
    return false;
}
