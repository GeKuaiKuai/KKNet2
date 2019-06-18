#include "PollSelect.h"
#include "EventHandler.h"

using namespace kknet;
int PollSelect::wait(int time)
{
    fd_set rset;
    fd_set wset;
    fd_set eset;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&eset);
    timeval val;
    val.tv_sec = time;
    val.tv_usec = 0;
    for(Events::iterator walk = events_.begin();walk!=events_.end();walk++)
    {
        if(walk->second->read)
            FD_SET(walk->second->fd,&rset);
        if(walk->second->write)
            FD_SET(walk->second->fd,&wset);
        if(walk->second->error)
            FD_SET(walk->second->fd,&eset);
    }

    int n = ::select(static_cast<int>(maxfd_+1),&rset,&wset,&eset,&val);
    
    srevents_.clear();
    for(Events::iterator walk = events_.begin();walk!=events_.end();walk++)
    {
        bool flag = false;
        if(FD_ISSET(walk->second->fd,&rset) && walk->second->read)
        {
            flag = true;
            walk->second->handler->setEventFlag(kRead);
        }
        if(FD_ISSET(walk->second->fd,&wset) &&  walk->second->write)
        {
            flag = true;
            walk->second->handler->setEventFlag(kWrite);
        }
        if(FD_ISSET(walk->second->fd,&eset) && walk->second->error)
        {
            flag = true;
            walk->second->handler->setEventFlag(kError);
        }
        if(flag)
        {
            srevents_.push_back(walk->second->handler);
        }
    }
    return n;
}

bool PollSelect::addFd(int fd,EventHandler* h)
{
    FdEvent* ptr = new FdEvent(h);
    ptr->fd = fd;
    if(fd > maxfd_)
        maxfd_ = fd;
    events_[fd] = std::unique_ptr<FdEvent>(ptr);
    return true;
}

bool PollSelect::setEvent(int fd,EventType type)
{
    if(!events_[fd])
    {
        show("fd不存在");
        return false;
    }
    if(type == EventType::kRead)
        events_[fd]->read = true;
    else if(type == EventType::kWrite)
        events_[fd]->write = true;
    else if(type == EventType::kError)
        events_[fd]->error = true;

}
bool PollSelect::deleteEvent(int fd,EventType type)
{
    if(!events_[fd])
    {
        show("fd不存在");
        return false;
    }
    if(type == EventType::kRead)
        events_[fd]->read = false;
    else if(type == EventType::kWrite)
        events_[fd]->write = false;
    else if(type == EventType::kError)
        events_[fd]->error = false;
    return true;
           
}

bool PollSelect::deleteFd(int fd)
{
    if(!events_[fd])
    {
        show("fd不存在");
        return false;
    }
    else
    {
        events_.erase(fd);
        if(maxfd_ == fd)
            --maxfd_;
        return true;
    }    
}