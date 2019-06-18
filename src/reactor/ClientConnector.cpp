#include "ClientConnector.h"
#include "Eventloop.h"
#include "../util/Socket.h"
#include "EventHandler.h"
using namespace kknet;


const int ClientConnector::kMaxRetryDelayMs;
ClientConnector::ClientConnector(Eventloop* loop,const string ip,int16_t port)
:loop_(loop),ip_(ip),port_(port),connect_(false),
state_(kDisconnected),retryDelayMs_(ClientConnector::kInitRetryDelayMs)
{

}

ClientConnector::~ClientConnector()
{

}

void ClientConnector::start()
{
    connect_ = true;
    loop_->runInLoop(std::bind(&ClientConnector::startInLoop,this));
}

void ClientConnector::startInLoop()
{
    if(connect_)
    {
        connect();
    }
    else
    {

    }
}

void ClientConnector::connect()
{
    sockfd_ = socket::createNonblockSocket();
    int ret = socket::connect(sockfd_,ip_,port_);
    int savedErrno = (ret == 0) ? 0 : errno;
    switch(savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting();
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry();
            break;
        default:
            printf("有错误，错误errno编号是：%d\n",savedErrno);
            socket::close(sockfd_);
    }
}

void ClientConnector::restart()
{
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void ClientConnector::connecting()
{
    setState(kConnecting);
    handler_.reset(new EventHandler("Client Handler"));
    handler_->setWriteCallback(
        std::bind(&ClientConnector::handleWrite,this)
    );
    handler_->setErrorCallback(
        std::bind(&ClientConnector::handleError,this)
    );
    loop_->addFd(sockfd_,handler_.get());
    loop_->setEvent(sockfd_,kWrite);
}

void ClientConnector::retry()
{
    socket::close(sockfd_);
    setState(kDisconnected);
    if (connect_)
    {
        loop_->runAfter(retryDelayMs_/1000.0,
                        std::bind(&ClientConnector::startInLoop, shared_from_this()));

        retryDelayMs_ = std::min(retryDelayMs_ * 2, ClientConnector::kMaxRetryDelayMs);
        if(retryCallback_)
            retryCallback_();
    }

    
}

void ClientConnector::removeAndResetChannel()
{
    loop_->deleteFd(sockfd_);
    loop_->queueInLoop(std::bind(&ClientConnector::resetChannel,this));
}

void ClientConnector::resetChannel()
{
    handler_.reset();
}

void ClientConnector::handleWrite()
{
    if(state_ == kConnecting)
    {
        removeAndResetChannel();
        int err = socket::getSocketError(sockfd_);
        if(err)
        {
            retry();
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd_);
            }
            else
            {
                socket::close(sockfd_);
            }   
        }
        
    }
}

void ClientConnector::handleError()
{
    if (state_ == kConnecting)
    {
        removeAndResetChannel();
        int err = socket::getSocketError(sockfd_);
        retry();
    }
}

void ClientConnector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&ClientConnector::stopInLoop, this));
}

void ClientConnector::stopInLoop()
{
  if (state_ == kConnecting)
  {
      setState(kDisconnected);
      removeAndResetChannel();
      socket::close(sockfd_);
  }
}