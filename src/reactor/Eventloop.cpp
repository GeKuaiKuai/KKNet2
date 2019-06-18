#include "Eventloop.h"
#include <signal.h>
using namespace kknet;
namespace
{
    class IgnoreSigPipe
    {
        public:
            IgnoreSigPipe()
            {
                ::signal(SIGPIPE, SIG_IGN);
            }
    };
    IgnoreSigPipe initObj;

    int createEventfd()
    {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0)
        {
            show("eventfd创建失败\n");
        }
        return evtfd;
    }
}
__thread Eventloop* t_loopInThisThread = 0;

Eventloop::Eventloop()
:
poll_(new PollSelect()),
threadId_(currentThread::tid()),
wakeupFd_(createEventfd()),
eventHandling_(false), 
quit_(false),
fdnums_(0),
timerQueue_(new TimerQueue(this))
{
    if(!t_loopInThisThread)
        t_loopInThisThread = this;
    else
        show("当前线程已经有eventloop");
    wakeupHandler_.setName("wakeup Handler");
    wakeupHandler_.setReadCallback(std::bind(&Eventloop::wakeupReadCallback,this));
    addFd(wakeupFd_,&wakeupHandler_);
    setEvent(wakeupFd_,EventType::kRead);
}

void Eventloop::loop()
{
    while(!quit_)
    {
        int n = poll_->wait(WAIT_TIME);
        
        
        if(n > 0)
        {
            const EventsList &handlers = poll_->getEventHanlderList();
            for(auto &h:handlers)
            {
                h->handleEvents();
            }
            
        }
        
        eventHandling_ = true;
        if(!tasks_.empty())
        {
            runTasks();
        }
        eventHandling_ = false;
        
    }
    
}

void Eventloop::runInLoop(NormalFunc&& func)
{
    if(isInLoopThread())
    {
        func();
    }
    else
    {
        queueInLoop(std::move(func));
    }
}

void Eventloop::queueInLoop(NormalFunc&& func)
{
    //如果当前正在等待poll返回，就要唤醒
    if(!isInLoopThread() || eventHandling_)
    {
        wakeup();
    }
    addTask(std::move(func));
}

void Eventloop::addTask(NormalFunc&& func)
{
    mutex_.lock();
    tasks_.emplace_back(std::move(func));
    mutex_.unlock();
}


void Eventloop::runTasks()
{
    std::vector<NormalFunc> taskings;
    mutex_.lock();
    taskings.swap(tasks_);
    mutex_.unlock();
    for(auto task:taskings)
    {
        task();
    }
}


TimerId Eventloop::runAt(Timestamp time, NormalFunc cb)
{
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId Eventloop::runAfter(double delay, NormalFunc cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId Eventloop::runEvery(double interval, NormalFunc cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void Eventloop::cancel(TimerId timerId)
{
  return timerQueue_->cancel(timerId);
}