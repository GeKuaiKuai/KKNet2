#ifndef KKNET_EVENTHANDLER_H
#define KKNET_EVENTHANDLER_H
#include "../util/typedef.h"
#include "../util/public.h"
#include <memory>
namespace kknet
{   

    struct FdEvent;
    class EventHandler
    {
        public:
            public:
                EventHandler(const string& name="default EventHanlder");
                void setReadCallback(NormalFunc func)
                {
                    readCallback_ = std::move(func);
                }
                void setWriteCallback(NormalFunc func)
                {
                    writeCallback_ = std::move(func);
                }
                void setErrorCallback(NormalFunc func)
                {
                    errorCallback_ = std::move(func);
                }
                void setCloseCallback(NormalFunc func)
                {
                    closeCallback_ = std::move(func);
                }
                void handleEvents();
                void setEventFlag(EventType type);  
                string getName() const
                {
                    return name_;
                }
                void setName(const string& name)
                {
                    name_ = name;
                }

        private:
            void clearEvent()
            {
                read_ = false;
                write_ = false;
                error_ = false;
                close_ = false;
            }
            NormalFunc readCallback_;
            NormalFunc writeCallback_;
            NormalFunc errorCallback_;
            NormalFunc closeCallback_;
            bool read_;
            bool write_;
            bool error_;
            bool close_;
            string name_;
            

    };

    struct FdEvent
    {
        FdEvent(EventHandler* h)
        :fd(0),read(false),write(false),error(false),close(false),handler(h)
        {

        }
        int fd;
        bool read;
        bool write;
        bool error;
        bool close;
        EventHandler *handler;
    };
}

#endif