#ifndef __EXECUTOR_H_GUARD__
#define __EXECUTOR_H_GUARD__

#include <functional>

namespace zyzio {
    namespace concurrent {
        //functional interface for any runner
        class runnable {
        public:
            virtual void run() = 0;
        };

        template<class R>
        class callable {
        public:
            virtual R call() = 0;
        };

        //functional interface for any executor
        class executor {
        public:
            virtual void execute(runnable& command) = 0;
        };

    }
}
#endif
