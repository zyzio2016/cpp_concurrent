#include "../headers/threadpool.h"

using namespace std;

namespace zyzio {
    namespace concurrent {

        void executor_service::execute(runnable& command) {
            impl->addToQueue([&command]() {command.run(); });
        }

        void executor_service::execute(function< void()> f) {
            impl->addToQueue(f);
        }

        void executor_service::shutdown() {
            impl->shutdown();
        }

        bool executor_service::isShutdown () {
            return impl->isShutdown();
        }
    }
}