#ifndef __EXECUTOR_H_GUARD__
#define __EXECUTOR_H_GUARD__

#include <functional>

namespace zyzio {
    namespace concurrent {

        class runnable {
        public:
            virtual ~runnable() {}
            virtual void run() = 0;
        };

        template<class T>
        class callable {
        public:
            virtual ~callable() {}
            virtual T call() = 0;
        };

        ///functional interface for any executor
        class executor {
        public:
            virtual ~executor() {}
            virtual void execute(runnable& command) = 0;
        };

    }
}
#endif
