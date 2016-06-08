#include <iostream>
#include <vector>
#include "executors.h"
#include "TestException.h"
#include "TestThreadRunner.h"

using namespace std;
using namespace zyzio::concurrent;

struct ConstTestRunner : public runnable {
    TestThreadRunner* task;
    void run() {
        task->run ();
    }
};

struct ConstTestCallable : public callable<int> {
    TestThreadCallable<int>* task;
    int call () {
        return task->call();
    }
};


bool ThreadPool_invoke() {
    enum { TASK_SIZE = 10 };

    TestThreadRunner runner;
    TestThreadCallable<int> caller;
    unique_ptr<executor_service> threadPool(executors::newSingleThreadExecutor());
    try {
        vector<ConstTestRunner> list;
        for (int i = 0; i < TASK_SIZE; i++) {
            list.push_back(ConstTestRunner ());
            list.back().task = &runner;
        }
        threadPool->invokeAllRunnable(list.begin(), list.end ());
        if (runner.count != TASK_SIZE)
            throw TestException("ThreadPool_invoke (invokeAllRunnable) failed.");

        vector<ConstTestCallable> list2;
        for (int i = 0; i < TASK_SIZE; i++) {
            list2.push_back(ConstTestCallable());
            list2.back().task = &caller;
        }
        threadPool->invokeAllCallable<vector<ConstTestCallable>::iterator,int>(list2.begin(), list2.end());
        if (caller.count != TASK_SIZE)
            throw TestException("ThreadPool_invoke (invokeAllCallable) failed.");


        vector<TestRunnable*> list3;
        for (int i = 0; i < TASK_SIZE; i++) {
            list3.push_back(new TestRunnable());
        }
        TestRunnable::counter = 0;
        threadPool->invokeAllRunnable(list3.begin(), list3.end(), true);
        this_thread::sleep_for(chrono::milliseconds(10));
        if (TestRunnable::counter != TASK_SIZE)
            throw TestException("ThreadPool_invoke (invokeAllRunnable removeAfter) failed.");

        vector<TestCallable< int>* > list4;
        for (int i = 0; i < TASK_SIZE; i++) {
            list4.push_back(new TestCallable< int> (2));
        }
        TestCallable< int>::counter = 0;
        threadPool->invokeAllCallable<vector<TestCallable< int>* >::iterator, int>(list4.begin(), list4.end(), true);
        this_thread::sleep_for(chrono::milliseconds(10));
        if (TestCallable< int>::counter != TASK_SIZE)
            throw TestException("ThreadPool_invoke (invokeAllCallable removeAfter) failed.");

        typedef function<void(void)> AnyCall;
        vector<AnyCall> list5;
        atomic_uint anyCounter = 0;
        for (int i = 0; i < TASK_SIZE; i++) {
            list5.push_back([&anyCounter]() { anyCounter++; });
        }
        threadPool->invokeAll<vector<AnyCall>::iterator, AnyCall, void>(list5.begin (), list5.end());
        if (anyCounter != TASK_SIZE)
            throw TestException("ThreadPool_invoke (invokeAll) failed.");


        list5.clear();
        anyCounter = 0;
        for (int i = 0; i < TASK_SIZE; i++) {
            list5.push_back([&anyCounter]() { 
                anyCounter++; 
                this_thread::sleep_for(chrono::milliseconds(2));
            });
        }
        threadPool->invokeAll<vector<AnyCall>::iterator, AnyCall, void, chrono::milliseconds>(list5.begin(), list5.end(), chrono::milliseconds (3));
        if (anyCounter > 2)
            throw TestException("ThreadPool_invoke (invokeAll timeout) failed.");
        this_thread::sleep_for(chrono::milliseconds(30));
        if (anyCounter != TASK_SIZE)
            throw TestException("ThreadPool_invoke (invokeAll timeout) failed after timeout.");

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
