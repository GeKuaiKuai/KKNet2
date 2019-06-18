#ifndef KKNET_COUTDOWN_H
#define KKNET_COUTDOWN_H
#include "Mutex.h"
#include "Condition.h"
namespace kknet
{
    class CountDownLatch
    {
        public:
            explicit CountDownLatch(int count);
            void wait();
            void countDown();
            int getCount() const;
            int setCount(int count)
            {
                count_ = count;
            }
        private:
            mutable MutexLock mutex_;
            Condition cond_;
            int count_;
    };
}


#endif