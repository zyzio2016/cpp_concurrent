#include "../headers/threadpool.h"

namespace zyzio {
    namespace concurrent {

        void executor_service::execute(runnable& command, bool deleteAfter) {
            impl->addToQueue(command, deleteAfter);
        }


        void executor_service::shutdown() {
            impl->shutdown();
        }

        bool executor_service::isShutdown () {
            return impl->isShutdown();
        }
    }
}