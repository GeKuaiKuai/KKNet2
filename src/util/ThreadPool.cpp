#include "ThreadPool.h"
using namespace kknet;
ThreadPool::ThreadPool(int size):threadSize_(size)
{

}

void ThreadPool::start()
{
    threads_.reserve(threadSize_);
    for(int i=0;i<threadSize_;i++)
    {
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runFunc,this)));
        threads_[i]->start();  
    }
}
void ThreadPool::runFunc()
{
    while(true)
    {
        NormalFunc func = queue_.take();
        func();
    }
}
