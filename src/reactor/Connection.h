#ifndef KKNET_CONNECTION_H
#define KKNET_CONNECTION_H
#include "EventHandler.h"
#include "../util/public.h"
#include "../util/typedef.h"
#include <stdint.h>
#include "../util/Socket.h"
#include <memory>
#include "KBuffer.h"
namespace kknet
{
    class KBuffer;
    class Eventloop;
    void defaultConnectionCallback(const ConnectionPtr& conn);
    void defaultReadCallback(const ConnectionPtr& conn);


    class Connection:public std::enable_shared_from_this<Connection>
    {
        public:
            Connection(Eventloop* loop,const string& name,int fd,sockaddr_in addr);
            ~Connection();
            Eventloop* getLoop() const
            {
                return loop_;
            }
            bool isConnected() const
            {
                return state_ == kConnected;
            }
            bool isDisconnected() const
            {
                return state_ == kDisconnected;
            }
            void send(const void* data,size_t len);
            void send(const string& data);
            void send(KBuffer* data);
            void shutdown();
            void forceClose();
            bool isReading() const
            {
                return reading_;
            }
            bool isWriting() const
            {
                return writing_;
            }
            uint16_t getPort() const
            {
                return port_;
            }
            string getIp() const
            {
                return ip_;
            }
            int getFd() const
            {
                return fd_;
            }

            void setWeakPtr(WeakTimeWheelEntryPtr ptr)
            {
                weakPtr_ = ptr;
            }

            WeakTimeWheelEntryPtr getWeakPtr() const
            {
                return weakPtr_;
            }
            
            void setConnectionCallback(ConnectionFunc cb)
            {
                connectionCallback_ = cb;
            }

            void setMessageCallback(ConnectionFunc cb)
            {
                messageCallback_ = cb;
            }
            
            void setWriteCompleteCallback(ConnectionFunc cb)
            {
                writeCompleteCallback_ = cb;
            }

            void setCloseCallback(ConnectionFunc cb)
            {
                closeCallback_ = cb;
            }
            
            KBuffer* getInputBuffer()
            {
                return (&inputBuffer_);                
            }
            
            KBuffer* getOutputBuffer()
            {
                return (&outputBuffer_);
            }

            //异步，建立连接收尾工作，设置监听事件
            void completeConnect();

            /*close事件执行时处于eventhandler函数栈，如果直接销毁connection会产生异常
            * 所以将connectedDistroyed绑定在task queue，下次执行tasks再销毁connection
            * 把Server removeConnection 改成queueInLoop，即可去掉这个步骤
            */
            void connectDestroyed();

        private:
            
            enum State
            {
                kDisconnected,
                kConnecting,
                kConnected,
                kDisconnecting
            };
            State state_;
            Eventloop* loop_;
            EventHandler eventHandler_;
            WeakTimeWheelEntryPtr weakPtr_;
            int fd_;
            int16_t port_;
            string ip_;
            string name_;
            bool reading_;
            bool writing_;
            //接收缓冲区
            KBuffer inputBuffer_;
            //发送缓冲区
            KBuffer outputBuffer_;
            //扩展数据
            void *data;
            ConnectionFunc connectionCallback_;
            ConnectionFunc messageCallback_;
            ConnectionFunc writeCompleteCallback_;
            ConnectionFunc closeCallback_;

            void handleRead();
            void handleWrite();
            void handleClose();
            void handleError();

            void sendInLoop(const void*data,size_t len);
            void shutdownInLoop();
            void enableReading();
            void disableReading();
            void enableWriting();
            void disableWriting();
            void setState(State e)
            {
                state_ = e;
            }
    };
    

}
#endif