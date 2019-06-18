#include "Connection.h"
#include "Eventloop.h"
using namespace kknet;
void kknet::defaultConnectionCallback(const ConnectionPtr& conn)
{
}

void kknet::defaultReadCallback(const ConnectionPtr& conn)
{
}

Connection::Connection(Eventloop* loop,const string& name,int fd,sockaddr_in addr)
:loop_(loop),name_(name),fd_(fd),state_(kConnecting),reading_(false)
{
    port_ = addr.sin_port;
    ip_ = socket::turnIpToString(&addr);
    socket::setKeepAlive(fd_,true);
    eventHandler_.setName("connection Handler");
    eventHandler_.setReadCallback(std::bind(&Connection::handleRead,this));
    eventHandler_.setWriteCallback(std::bind(&Connection::handleWrite,this));
    eventHandler_.setCloseCallback(std::bind(&Connection::handleClose,this));
}

Connection::~Connection()
{
    socket::close(fd_);
}

void Connection::completeConnect()
{
    
    loop_->addFd(fd_,&eventHandler_);
    enableReading();
    setState(kConnected);
    if(connectionCallback_)
        connectionCallback_(shared_from_this());
}

void Connection::enableReading()
{
    reading_ = true;
    loop_->setEvent(fd_,kRead);   
}

void Connection::disableReading()
{
    reading_ = false;
    loop_->deleteEvent(fd_,kRead);
}

void Connection::enableWriting()
{
    writing_ = true;
    loop_->setEvent(fd_,kWrite);  
}

void Connection::disableWriting()
{
    writing_ = false;
    loop_->deleteEvent(fd_,kWrite);
}


void Connection::handleRead()
{
    ssize_t n = inputBuffer_.readFd(fd_);
    if(n > 0)
    {
        if(messageCallback_)
            messageCallback_(shared_from_this());
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        if(errno != EAGAIN)
        {
            handleClose();
        }
    }
}

void Connection::handleClose()
{
    if(state_ == kDisconnecting)
        setState(kDisconnected);
    loop_->deleteAllEvent(fd_);
    if(closeCallback_)
        closeCallback_(shared_from_this());
}

void Connection::connectDestroyed()
{
    loop_->deleteFd(fd_);
}

void Connection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&Connection::shutdownInLoop,shared_from_this()));
    }
}

void Connection::shutdownInLoop()
{
    if(!isWriting())
    {
        socket::shutdownWrite(fd_);
    }
}

void Connection::forceClose()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&Connection::handleClose,shared_from_this()));
    }
}

void Connection::send(const void* data,size_t len)
{
    loop_->runInLoop(std::bind(&Connection::sendInLoop,shared_from_this(),data,len));
}
void Connection::send(const string& data)
{
    send(data.c_str(),data.size());
}
void Connection::send(KBuffer* data)
{
   string d = data->readAllAsString();
   send(d);
}

void Connection::sendInLoop(const void*data,size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(state_ == kDisconnected)
    {
        return;
    }
    if(!writing_ && outputBuffer_.getReadableSize() == 0)
    {
        nwrote = socket::write(fd_,data,len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if(errno == EPIPE || errno == ECONNRESET)
            {
                faultError = true;
            }
        }

        if(!faultError && remaining > 0)
        {
            outputBuffer_.write(data+nwrote,remaining);
            enableWriting();
        }
    }
}

void Connection::handleWrite()
{
    if(writing_)
    {
        string data = outputBuffer_.peekAllAsString();
        ssize_t n = socket::write(fd_,data.c_str(),data.length());
        if(n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.getReadableSize() == 0)
            {
                disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
                if(state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
    }
}