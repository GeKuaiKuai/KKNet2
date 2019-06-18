#include "Acceptor.h"
#include "../util/Socket.h"
using namespace kknet;
Acceptor::Acceptor(Eventloop* loop,int16_t port,bool reusePort)
:loop_(loop),fd_(0),listening_(false)
{
    fd_ = socket::createNonblockSocket();
    if(reusePort)
        socket::setReuseAddr(fd_,true);
    socket::bind(fd_,port,false);
    handler_.setReadCallback(std::bind(&Acceptor::handleRead,this)); 
    handler_.setName("acceptor Handler");
}

Acceptor::~Acceptor()
{
    //这里要删除监听事件
    loop_->deleteFd(fd_);
}

void Acceptor::listen()
{
    socket::listen(fd_);

    loop_->addFd(fd_,&handler_);
    loop_->setEvent(fd_,EventType::kRead);
    listening_ = true;
}

void Acceptor::handleRead()
{
    ::sockaddr_in peer;
    int newfd = socket::accept(fd_,&peer);
    if(newfd >= 0)
    {
        if(newConnectionCallback_)
            newConnectionCallback_(newfd,peer);
    }
    else
    {
        socket::close(newfd);
    }
}
