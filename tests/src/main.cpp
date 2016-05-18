bool fixedThreadPool_execute();
bool fixedThreadPool_shutdown();

int main() { 
    int count = 0;
    if (!fixedThreadPool_execute()) count++;
    if (!fixedThreadPool_shutdown()) count++;
    return -count;
}