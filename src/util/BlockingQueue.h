#ifndef KKNET_BLOCKINGQUEUE_H
#define KKNET_BLOCKINGQUEUE_H
#include "Condition.h"
#include "Mutex.h"
#include <deque>
namespace kknet
{
    template<typename T>
    class BlockingQueue
    {
        public:
            BlockingQueue():mutex_(),notEmpty_(mutex_),notFull_(mutex_),maxQueueSize_(0){}
            void put(const T& x)
            {
                mutex_.lock();
                if(maxQueueSize_)
                {
                    while(queue_.size() >= maxQueueSize_)
                    {
                        notFull_.wait();
                    }
                }

                queue_.push_back(x);
                notEmpty_.notify();
                mutex_.unlock();
            }
            T take()
            {
                mutex_.lock();
                while(queue_.empty())
                {
                    notEmpty_.wait();
                }
                T front(queue_.front());
                queue_.pop_front();
                if(maxQueueSize_)
                    notFull_.notify();
                mutex_.unlock();
                return front;
            }
            void setMaxQueueSize(int32_t size)
            {
                maxQueueSize_ = size;
            }
            int getQueueSize() const
            {
                return queue_.size();
            }
        private:
            std::deque<T> queue_;
            /* maxQueueSize为-1时，表示阻塞队列不限制大小 */
            int32_t maxQueueSize_;
            MutexLock mutex_;
            Condition notEmpty_;
            Condition notFull_;
    };
}

#endif