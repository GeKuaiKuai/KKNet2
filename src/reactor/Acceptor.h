#ifndef KKNET_ACCEPTOR_H
#define KKNET_ACCEPTOR_H
#include "Eventloop.h"
#include "../util/typedef.h"
#include <stdint.h>
namespace kknet
{
    class Acceptor
    {
        public:
            Acceptor(Eventloop* loop,int16_t port,bool reusePort);
            ~Acceptor();
            void setNewConnectionCallback(const NewConnectionFunc& fun)
            {
                newConnectionCallback_ = fun;
            }
            void listen();
            bool isListening() const
            {
                return listening_;
            }
        private:
            void handleRead();
            Eventloop* loop_;
            int fd_;
            bool listening_;
            EventHandler handler_;
            NewConnectionFunc newConnectionCallback_;
    };
}
#endif