#include "..\headers\executors.h"
#include "fixedthreadpool.h"

namespace zyzio {
    namespace concurrent {
        namespace executors {

            executor_service* newFixedThreadPool(int nThreads) {
                return new executor_service(new fixed_thread_pool(nThreads));
            }

        }
    }
}
