bool fixedThreadPool_execute();
bool fixedThreadPool_execute_lambda();
bool ThreadPool_submit_lambda();
bool ThreadPool_shutdown();

int main() { 
    int failedCount = 0;
    if (!fixedThreadPool_execute()) failedCount++;
    if (!fixedThreadPool_execute_lambda()) failedCount++;
    if (!ThreadPool_submit_lambda()) failedCount++;
    if (!ThreadPool_shutdown()) failedCount++;
    return -failedCount;
}