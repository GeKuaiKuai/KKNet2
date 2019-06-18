#ifndef KKNET_SELECT_H
#define KKNET_SELECT_H
#include "Poll.h"
#include "../util/public.h"
#include <sys/select.h>
namespace kknet
{
    class PollSelect : public Poll
    {
        public:
            PollSelect(){};
            virtual int wait(int time);
            virtual bool addFd(int fd,EventHandler*);
            virtual bool deleteFd(int fd);

            virtual bool setEvent(int fd,EventType type);
            virtual bool deleteEvent(int fd,EventType type);
            virtual ~PollSelect()
            {
                
            }
        private:
            int maxfd_;

    };
}
#endif