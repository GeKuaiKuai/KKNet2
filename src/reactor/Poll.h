#ifndef KKNET_POLL_H
#define KKNET_POLL_H
/*
 * 纯虚函数，三种io复用的接口
*/
#include <vector>
#include <map>
#include "EventHandler.h"

namespace kknet
{
    typedef std::map<int,std::unique_ptr<FdEvent> > Events;
    typedef std::vector<EventHandler*> EventsList;

    class Poll
    {
        public:

            Poll():num_(0){};
            //单位 秒
            virtual int wait(int time) = 0;
            virtual bool addFd(int fd,EventHandler* h) = 0;
            virtual bool deleteFd(int fd) = 0;
            virtual bool setEvent(int fd,EventType type) = 0;
            virtual bool deleteEvent(int fd,EventType type) = 0;
            virtual ~Poll()
            {

            }

            const EventsList& getEventHanlderList()
            {
                return srevents_;
            }
        protected:

            Events events_;
            EventsList srevents_;
            int num_;
    };
}
#endif