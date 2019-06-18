#ifndef KKNET_CONDITION_H
#define KKNET_CONDITION_H
#include "Mutex.h"
namespace kknet
{
    class Condition
    {
        public:
            explicit Condition(MutexLock& mutex)
            :mutex_(mutex)
            {
                pthread_cond_init(&cond_,NULL);
            }
            ~Condition()
            {
                pthread_cond_destroy(&cond_);
            }
            void wait()
            {
                mutex_.unsetHolder();
                pthread_cond_wait(&cond_,mutex_.getMutex());
                mutex_.setHolder();
            }
            void notify()
            {
                pthread_cond_signal(&cond_);
            }

            void notifyAll()
            {
                pthread_cond_broadcast(&cond_);
            }
        private:
            MutexLock &mutex_;
            pthread_cond_t cond_;
    };
}
#endif