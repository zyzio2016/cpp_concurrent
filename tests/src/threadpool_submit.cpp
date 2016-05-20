#include <iostream>
#include <thread>
#include "executors.h"
#include "TestThreadRunner.h"

using namespace std;
using namespace zyzio::concurrent;

bool ThreadPool_submit_lambda() {
    try {
        unique_ptr<executor_service> threadPool(executors::newSingleThreadExecutor());
        auto f = threadPool->submit([](int x) { this_thread::sleep_for(chrono::seconds(x)); return x; }, 2);
        future_status stat = f.wait_for(chrono::seconds(1));
        if (stat != future_status::timeout)
            throw exception("ThreadPool_submit_lambda failed. Future.wait_for failed.");
        f.get();
        cout << "ThreadPool_submit_lambda succeded" << endl;
        return true;
    } catch (exception& e) {
        cerr << "ThreadPool_submit_lambda failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "ThreadPool_submit_lambda failed. Reason unknown." << endl;
    }
    return false;
}

template<class T>
class TestCallable : public callable<T> {
public:
    static atomic_uint counter;
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

atomic_uint TestCallable<int>::counter;
atomic_uint TestCallable<double>::counter;

class TestRunnable : public runnable {
public:
    static atomic_uint counter;
    void run() {}
    ~TestRunnable() {
        counter++;
    }
};

atomic_uint TestRunnable::counter;

bool ThreadPool_submit() {
    enum { DESTRUCTOR_COUNT = 6 };

    try {
        unique_ptr<executor_service> threadPool(executors::newSingleThreadExecutor());
        TestCallable<double> c (1010);
        auto cf = threadPool->submitCallable(c);
        future_status stat = cf.wait_for(chrono::seconds(1));
        if (stat != future_status::ready)
            throw exception("ThreadPool_submit failed. Future.wait_for for callable failed.");
        if (cf.get() != 1010.0) {
            throw exception("ThreadPool_submit failed. Callable should return 1010.0.");
        }

        TestRunnable r;
        auto rf = threadPool->submitRunnable(r);
        stat = rf.wait_for(chrono::seconds(1));
        if (stat != future_status::ready)
            throw exception("ThreadPool_submit failed. Future.wait_for for runnable failed.");

        for (int i = 0; i < DESTRUCTOR_COUNT; i++) {
            threadPool->submitCallable(&c);
            if (threadPool->submitCallable(new TestCallable<int>(i), true).get () != i)
                throw exception("ThreadPool_submit failed. Calleble should return i.");
            threadPool->submitRunnable(&r);
            threadPool->submitRunnable(new TestRunnable(), true).get();
        }
        if (TestRunnable::counter != DESTRUCTOR_COUNT) {
            throw new exception("ThreadPool_submit: wrong number of DesctructorCounter were freed");
        }
        if (TestCallable<int>::counter != DESTRUCTOR_COUNT) {
            throw new exception("ThreadPool_submit: wrong number of TestCallable<int> were freed");
        }

        cout << "ThreadPool_submit succeded" << endl;
        return true;
    } catch (exception& e) {
        cerr << "ThreadPool_submit failed." << endl;
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "ThreadPool_submit failed. Reason unknown." << endl;
    }
    return false;
}