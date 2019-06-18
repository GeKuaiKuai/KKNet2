#include "EventloopPool.h"
#include <algorithm>

using namespace kknet;

namespace
{
    bool compare(Eventloop* a,Eventloop* b)
    {
        return (a->getFdNums()) > (b->getFdNums());
    }
}

EventloopPool::EventloopPool(Eventloop* loop)
:num_(1),
currentLoop_(0),
latch_(1),
robinNum_(0)
{
    loops_.push_back(loop);
}
EventloopPool::~EventloopPool()
{
    //设置quit标志，让eventloop循环退出
    for(auto l:loops_)
    {
       l->setQuit();
    }
    //thread使用unique_ptr管理，自动调用析构函数，无需手动pthread_datach
}
void EventloopPool::start()
{
    for(int i=1;i<num_;i++)
    {
        currentLoop_ = 0;
        std::unique_ptr<Thread> t = std::unique_ptr<Thread>(new Thread(std::bind(&EventloopPool::poolFunc,this)));
        t->start();
        //等待线程中的局部eventloop构造完成，地址存储到currentLoop_中
        latch_.wait();
        loops_.push_back(currentLoop_);
        threads_.push_back(std::move(t));
        latch_.setCount(1);
    }
}

Eventloop* EventloopPool::getNextEvent()
{
    //连接过多时使用sort排序选择，否则使用round-robin
    if(loops_[0]->getFdNums() > MAX_ROUND_ROBIN_NUM)
    {
        std::sort(loops_.begin(),loops_.end(),compare);
        return loops_[0];
    }
    else
    {
        if(robinNum_ >= num_)
            robinNum_ = 0;
        return loops_[robinNum_];
    }

}

void EventloopPool::poolFunc()
{
    Eventloop loop;
    currentLoop_ = &loop;
    latch_.countDown();
    loop.loop();
}