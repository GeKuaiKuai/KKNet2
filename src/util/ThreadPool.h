#ifndef KKNET_THREADPOOL_H
#define KKNET_THREADPOOL_H
#include "BlockingQueue.h"
#include "Thread.h"
#include <vector>
#include <memory>
namespace kknet
{
    class ThreadPool
    {
        public:
            ThreadPool(int size);
            void setMaxQueueSize(int size)
            {
                queue_.setMaxQueueSize(size);
            }
            int getQueueSize() const
            {
                return queue_.getQueueSize();
            }
            void put(const NormalFunc& func)
            {
                queue_.put(func);
            }
            void start();
            
        private:
            void runFunc();
            BlockingQueue<NormalFunc> queue_;
            std::vector<std::unique_ptr<Thread> > threads_;
            int threadSize_;
    };
}

#endif