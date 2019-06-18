#ifndef KKNET_CLIENT_H
#define KKNET_CLIENT_H
#include "../util/Mutex.h"
#include "Connection.h"
#include "Eventloop.h"

namespace kknet
{
    class ClientConnector;
    typedef std::shared_ptr<ClientConnector> ConnectorPtr;

    void removeConnection(Eventloop* loop,const ConnectionPtr& conn);

    class Client
    {
        public:
        Client(Eventloop* loop,const string& ip,int16_t port,const string& nameArg);
        ~Client();

        void connect();
        void disconnect();
        void stop();

        ConnectionPtr getConnection()
        {
            return connection_;
        }

        Eventloop* getLoop() const { return loop_; }

        bool retry() const { return retry_; }
        void enableRetry() { retry_ = true; }

        const string& getName() const
        {
            return name_;
        }

        bool isConnected() const;

        void setConnectionCallback(ConnectionFunc cb)
        { connectionCallback_ = std::move(cb); }

        void setMessageCallback(ConnectionFunc cb)
        { messageCallback_ = std::move(cb); }

        void setWriteCompleteCallback(ConnectionFunc cb)
        { writeCompleteCallback_ = std::move(cb); }



        void setRetryCallback(NormalFunc cb);

        private:
        void newConnection(int sockfd);
        void removeConnection(const ConnectionPtr& conn);

        Eventloop* loop_;
        ConnectorPtr connector_;
        const string& ip_;
        int16_t port_;
        const string name_;
        ConnectionFunc connectionCallback_;
        ConnectionFunc messageCallback_;
        ConnectionFunc writeCompleteCallback_;
        bool retry_;
        bool connect_;
        int nextConnId_;
        MutexLock mutex_;
        ConnectionPtr connection_;

    };
}
#endif