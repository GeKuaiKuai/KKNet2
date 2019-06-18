#ifndef KKNET_EVENTLOOP_H
#define KKNET_EVENTLOOP_H
#include "Poll.h"
#include "PollSelect.h"
#include "../util/typedef.h"
#include <memory>
#include <vector>
#include "../util/Thread.h"
#include <sys/eventfd.h>
#include "../util/Socket.h"
#include "../util/Mutex.h"
#include "TimeEvent.h"

#define WAIT_TIME 5
namespace kknet
{


    class Eventloop
    {
        public:
            Eventloop();

            void loop();
            
            void runInLoop(NormalFunc&& func);
            void queueInLoop(NormalFunc&& func);
            void addTask(NormalFunc&& func);
            void runTasks();

            void addFd(int fd,EventHandler* h)
            {
                poll_->addFd(fd,h);
                ++fdnums_;
            }

            void deleteFd(int fd)
            {
                poll_->deleteFd(fd);
                --fdnums_;
            }

            void setEvent(int fd,EventType type)
            {
                poll_->setEvent(fd,type);
            }

            void deleteEvent(int fd,EventType type)
            {
                poll_->deleteEvent(fd,type);
            }

            void deleteAllEvent(int fd)
            {
                poll_->deleteEvent(fd,kClose);
                poll_->deleteEvent(fd,kError);
                poll_->deleteEvent(fd,kRead);
                poll_->deleteEvent(fd,kWrite);
            }


            void wakeupReadCallback()
            {
                uint64_t one = 1;
                socket::read(wakeupFd_, &one, sizeof one);
            }

            void wakeup()
            {
                uint64_t one = 1;
                socket::write(wakeupFd_, &one, sizeof one);               
            }
            void setQuit()
            {
                quit_ = true;
            }

            int32_t getFdNums() const
            {
                return fdnums_;
            }

            TimerId runAt(Timestamp time,NormalFunc cb);
            TimerId runAfter(double delay,NormalFunc cb);
            TimerId runEvery(double interval,NormalFunc cb);
            void cancel(TimerId timerId);

        private:
            bool isInLoopThread()
            {
                return threadId_ == currentThread::tid();
            }
            std::unique_ptr<Poll> poll_;
            std::vector<NormalFunc> tasks_;
            pid_t threadId_;
            std::unique_ptr<TimerQueue> timerQueue_;
            bool eventHandling_;
            int wakeupFd_;
            EventHandler wakeupHandler_;
            MutexLock mutex_;
            bool quit_;
            int32_t fdnums_; //当前eventloop fd数目，用于负载均衡 


    };
}
#endif