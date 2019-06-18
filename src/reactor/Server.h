#ifndef KKNET_SERVER_H
#define KKNET_SERVER_H
#include <stdint.h>
#include "../util/Socket.h"
#include "../util/public.h"
#include "../util/typedef.h"
#include <map>
namespace kknet
{
    class Eventloop;
    class Acceptor;
    class EventloopPool;
    class Server
    {
        public:
            Server(Eventloop* loop,const string& name,int16_t port,bool reusePort);
            ~Server();
            void setThreadNum(int num);
            void start();
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
        private:
            void newConnection(int sockfd,sockaddr_in addr);
            void removeConnection(const ConnectionPtr& conn);
            void removeConnectionInLoop(const ConnectionPtr& conn);
            typedef std::map<int, ConnectionPtr> ConnectionMap;
            ConnectionFunc connectionCallback_;
            ConnectionFunc messageCallback_;
            ConnectionFunc writeCompleteCallback_;
            ConnectionFunc closeCallback_;
            Eventloop* loop_;
            const string name_;
            int16_t port_;
            bool reusePort_;
            std::unique_ptr<Acceptor> acceptor_;
            std::unique_ptr<EventloopPool> threadPool_;
            int nextConnId_;
            ConnectionMap connections_;
    };
}
#endif