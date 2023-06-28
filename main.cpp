#include "ThreadPool.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

void taskFunc(void* arg)
{
    int nNum = *(int*)arg;
    cout << "thread: " << std::this_thread::get_id() << ", number=" << nNum << endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
int main()
{
    // 设置线程池最小5个线程，最大10个线程
    ThreadPool pool(5, 10);
    int i;
    // 往任务队列中添加100个任务
    for (i = 0; i < 10; ++i)
    {
        int* pNum = new int(i + 10);
        pool.Add(taskFunc, (void*)pNum);
    }
    for (; i < 20; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int* pNum = new int(i + 10);
        pool.Add(taskFunc, (void*)pNum);
    }
    return 0;
}
