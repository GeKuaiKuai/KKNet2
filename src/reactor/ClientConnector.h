#ifndef KKNET_CONNECTOR_H
#define KKNET_CONNECTOR_H
#include "../util/typedef.h"
#include "EventHandler.h"
#include <memory>

namespace kknet
{
    class Eventloop;
    class EventHandler;
    class ClientConnector: public std::enable_shared_from_this<ClientConnector>
    {
    public:
        typedef std::function<void(int sockfd)> NewConnectionCallback;
        ClientConnector(Eventloop* loop,const string ip,int16_t port);
        ~ClientConnector();

        void setNewConnectionCallback(const NewConnectionCallback& cb)
        { newConnectionCallback_ = cb; }

        void start();
        void restart();
        void stop();

        void setRetryCallback(NormalFunc cb)
        {
            retryCallback_ = cb;
        }

        bool isConnected()
        {
            return state_ == States::kConnected;
        }

        string getIp()
        {
            return ip_;
        }
        int16_t getPort()
        {
            return  port_;
        }

    private:
        enum States
        {
            kDisconnected,
            kConnecting,
            kConnected
        };
        static const int kMaxRetryDelayMs = 30*1000;
        static const int kInitRetryDelayMs = 500;

        Eventloop* loop_;
        const string ip_;
        const int16_t port_;

        bool connect_;
        States state_;
        int sockfd_;
        std::unique_ptr<EventHandler> handler_;
        NewConnectionCallback newConnectionCallback_;
        NormalFunc retryCallback_;

        int retryDelayMs_;

        void setState(States s){state_ = s;}
        void startInLoop();
        void stopInLoop();
        void connect();
        void connecting();
        void handleWrite();
        void handleError();
        void retry();
        void removeAndResetChannel();
        void resetChannel();

    };
}

#endif