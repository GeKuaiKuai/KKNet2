#ifndef KKNET_EVENTLOOPPOOL_H
#define KKNET_EVENTLOOPPOOL_H

#include "../util/Thread.h"
#include "Eventloop.h"
#include <vector>
#include "../util/CountDownLatch.h"

#define MAX_ROUND_ROBIN_NUM 10
namespace kknet
{
    class EventloopPool
    {
        public:
            EventloopPool(Eventloop* loop);
            ~EventloopPool();
            void setThreadNum(int n)
            {
                num_ = n;
            }
            void start();
            Eventloop* getNextEvent();
        private:
            void poolFunc();
            //eventloop是线程栈局部变量，无需使用智能指针
            std::vector<Eventloop*> loops_;
            std::vector<std::unique_ptr<Thread> > threads_;
            int num_;
            int robinNum_;
            Eventloop* currentLoop_;
            CountDownLatch latch_;
    };
}

#endif

