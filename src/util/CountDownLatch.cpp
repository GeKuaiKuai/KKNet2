#include "CountDownLatch.h"
using namespace kknet;
CountDownLatch::CountDownLatch(int count)
:mutex_(),cond_(mutex_),count_(count)
{
}

void CountDownLatch::wait()
{
    mutex_.lock();
    while(count_ > 0)
    {
        cond_.wait();
    }
    mutex_.unlock();
}

void CountDownLatch::countDown()
{
    mutex_.lock();
    --count_;
    if(count_ == 0)
    {
        cond_.notifyAll();
    }
    mutex_.unlock();
}

int CountDownLatch::getCount() const
{
    return count_;
}