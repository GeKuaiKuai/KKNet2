#ifndef KKNET_MUTEX_H
#define KKNET_MUTEX_H
#include <pthread.h>
#include "Thread.h"
namespace kknet
{
    class MutexLock
    {
        public:
            MutexLock()
            :holder_(0)
            {
                pthread_mutex_init(&mutex_,NULL);
            }
            ~MutexLock()
            {
                pthread_mutex_destroy(&mutex_);
            }
            void lock()
            {
                pthread_mutex_lock(&mutex_);
                setHolder(); 
            }
            void unlock()
            {
                pthread_mutex_unlock(&mutex_);
                unsetHolder();
            }
            void setHolder()
            {
                holder_ = currentThread::tid();
            }
            void unsetHolder()
            {
                holder_ = 0;
            }
            pthread_mutex_t* getMutex()
            {
                return &mutex_;
            }

        private:
            pthread_mutex_t mutex_;
            pid_t holder_;
    };

}

#endif