#ifndef KKNET_TYPEDEF_H
#define KKNET_TYPEDEF_H
#include <functional>
#include <memory>
class sockaddr_in;

namespace kknet
{   
    class Connection;
    class TimeWheelEntry;
    typedef std::function<void()> NormalFunc;
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::weak_ptr<Connection> WeakConnectionPtr;
    typedef std::weak_ptr<TimeWheelEntry> WeakTimeWheelEntryPtr;
    typedef std::function<void(const ConnectionPtr&)> ConnectionFunc;
    typedef std::function<void(int,::sockaddr_in)> NewConnectionFunc;

    typedef enum
    {
        kRead,
        kWrite,
        kError,
        kClose
    }EventType;
}

#endif