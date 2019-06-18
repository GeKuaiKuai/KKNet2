#ifndef KKNET_TIMEEVENT_H
#define KKNET_TIMEEVENT_H
#include "../util/Timestamp.h"
#include "../util/typedef.h"
#include "EventHandler.h"
#include <set>
#include <vector>
namespace kknet
{
    class Timer
    {
        public:
        Timer(NormalFunc cb,Timestamp time,double interval)
        :callback_(std::move(cb)),time_(time),interval_(interval),
        repeat_(interval>0.0),sequence_(++num_)
        {

        }

        void run() const
        {
            callback_();
        }

        Timestamp getTime() const
        {
            return time_;
        }

        bool repeat() const { return repeat_; }

        int64_t sequence() const { return sequence_; }

        void restart(Timestamp now);

        static int64_t getNum()
        {
            return num_;
        }


        private:
        const NormalFunc callback_;
        Timestamp time_;
        const double interval_;
        const bool repeat_;
        const int64_t sequence_;

        static int64_t num_;
    };

    class TimerId
    {
        public:
        TimerId()
            : timer_(NULL),
            sequence_(0)
        {
        }

        TimerId(Timer* timer, int64_t seq)
            : timer_(timer),
            sequence_(seq)
        {
        }
        bool isEmpty() const
        {
            return timer_ == NULL;
        }
        friend class TimerQueue;

        private:
        Timer* timer_;
        int64_t sequence_;        
    };

    class Eventloop;

    class TimerQueue
    {
        public:
          explicit TimerQueue(Eventloop* loop);
            ~TimerQueue();
            TimerId addTimer(NormalFunc cb,
                            Timestamp when,
                            double interval);
            void cancel(TimerId timerId);
        private:
            typedef std::pair<Timestamp, Timer*> Entry;
            typedef std::set<Entry> TimerList;
            typedef std::pair<Timer*, int64_t> ActiveTimer;
            typedef std::set<ActiveTimer> ActiveTimerSet;

            void addTimerInLoop(Timer* timer);
            void cancelInLoop(TimerId timerId);
            void handleRead();
            std::vector<Entry> getExpired(Timestamp now);
            void reset(const std::vector<Entry>& expired, Timestamp now);
            bool insert(Timer* timer);

            Eventloop* loop_;
            const int timerfd_;
            EventHandler timerfdHandler_;
            TimerList timers_;
            ActiveTimerSet activeTimers_;
            bool callingExpiredTimers_;
            ActiveTimerSet cancelingTimers_;


    };
}

#endif
