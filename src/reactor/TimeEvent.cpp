#include "TimeEvent.h"
#include "../util/public.h"
#include "Eventloop.h"

#include <sys/timerfd.h>
#include <unistd.h>
namespace kknet
{
    int64_t Timer::num_;

    void Timer::restart(Timestamp now)
    {
        if (repeat_)
        {
            time_ = addTime(now, interval_);
        }
        else
        {
            time_ = Timestamp::invalid();
        }
    }

    int createTimerfd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                        TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0)
        {
            printf("[error]Timer创建失败\n");
        }
        return timerfd;
    }

    struct timespec howMuchTimeFromNow(Timestamp when)
    {
        int64_t microseconds = when.getMicroSecondsSinceEpoch()
                                - Timestamp::now().getMicroSecondsSinceEpoch();
        if(microseconds < 100)
        {
            microseconds = 100;
        }

        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(microseconds/Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>((microseconds%Timestamp::kMicroSecondsPerSecond)*1000);
        return ts;
    }

    void readTimerfd(int timerfd,Timestamp now)
    {
        uint64_t howmany;
        ssize_t n = ::read(timerfd, &howmany, sizeof howmany);

    }

    void resetTimerfd(int timerfd, Timestamp expiration)
    {
        struct itimerspec newValue;
        struct itimerspec oldValue;
        memInit(&newValue, sizeof newValue);
        memInit(&oldValue, sizeof oldValue);
        newValue.it_value = howMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    }

    TimerQueue::TimerQueue(Eventloop* loop)
    :loop_(loop),timerfd_(createTimerfd()),timerfdHandler_("Timer Handler"),
        timers_(),callingExpiredTimers_(false)
    {
          timerfdHandler_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
          loop_->addFd(timerfd_,&timerfdHandler_);
          loop_->setEvent(timerfd_,kRead);


    }

    TimerQueue::~TimerQueue()
    {

        loop_->deleteFd(timerfd_);
        ::close(timerfd_);
        for (const Entry& timer : timers_)
        {
            delete timer.second;
        }
    }

    TimerId TimerQueue::addTimer(NormalFunc cb,Timestamp when,double interval)
    {
        Timer* timer = new Timer(std::move(cb), when, interval);
        loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
        return TimerId(timer,timer->sequence());
    }

    void TimerQueue::cancel(TimerId timerId)
    {
        loop_->runInLoop(
            std::bind(&TimerQueue::cancelInLoop, this, timerId));
    }

    void TimerQueue::addTimerInLoop(Timer* timer)
    {
        bool earliestChanged = insert(timer);
        if (earliestChanged)
        {
            resetTimerfd(timerfd_, timer->getTime());
        }        
    }

    void TimerQueue::cancelInLoop(TimerId timerId)
    {

        ActiveTimer timer(timerId.timer_, timerId.sequence_);
        ActiveTimerSet::iterator it = activeTimers_.find(timer);
        if (it != activeTimers_.end())
        {
            size_t n = timers_.erase(Entry(it->first->getTime(), it->first));
            delete it->first;
            activeTimers_.erase(it);
        }
        else if (callingExpiredTimers_)
        {
            cancelingTimers_.insert(timer);
        }

    }

    void TimerQueue::handleRead()
    {
        Timestamp now(Timestamp::now());
        readTimerfd(timerfd_, now);
        std::vector<Entry> expired = getExpired(now);
        callingExpiredTimers_ = true;
        cancelingTimers_.clear();
        for (const Entry& it : expired)
        {
            it.second->run();
        }
        callingExpiredTimers_ = false;
        reset(expired, now);
        
    }

    std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
    {
        std::vector<Entry> expired;
        Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
        TimerList::iterator end = timers_.lower_bound(sentry);
        std::copy(timers_.begin(), end, back_inserter(expired));
        timers_.erase(timers_.begin(), end);

        for (const Entry& it : expired)
        {
            ActiveTimer timer(it.second, it.second->sequence());
            size_t n = activeTimers_.erase(timer);
        }

        return expired;
    }

    void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
    {
        Timestamp nextExpire;

        for (const Entry& it : expired)
        {
            ActiveTimer timer(it.second, it.second->sequence());
            if (it.second->repeat()
                && cancelingTimers_.find(timer) == cancelingTimers_.end())
            {
                it.second->restart(now);
                insert(it.second);
            }
            else
            {
                // FIXME move to a free list
                delete it.second; // FIXME: no delete please
            }
        }

        if (!timers_.empty())
        {
            nextExpire = timers_.begin()->second->getTime();
        }

        if (nextExpire.valid())
        {
            resetTimerfd(timerfd_, nextExpire);
        }
    }

    bool TimerQueue::insert(Timer* timer)
    {
        bool earliestChanged = false;
        Timestamp when = timer->getTime();
        TimerList::iterator it = timers_.begin();
        if (it == timers_.end() || when < it->first)
        {
            earliestChanged = true;
        }
        {
            std::pair<TimerList::iterator, bool> result
            = timers_.insert(Entry(when, timer));
        }
        {
            std::pair<ActiveTimerSet::iterator, bool> result
            = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        }

        return earliestChanged;
    }
}
