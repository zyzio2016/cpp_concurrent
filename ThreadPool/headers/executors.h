#ifndef __EXECUTORS_H_GUARD__
#define __EXECUTORS_H_GUARD__

#include "threadpool.h"

namespace zyzio {
    namespace concurrent {
        namespace executors {
            executor_service* newFixedThreadPool(int nThreads);
        }
    }
}
#endif

