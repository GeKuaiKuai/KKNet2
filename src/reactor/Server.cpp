#include "Server.h"
#include "EventloopPool.h"
#include "Acceptor.h"
#include "Connection.h"
using namespace kknet;
Server::Server(Eventloop* loop,const string& name,int16_t port,bool reusePort)
:name_(name),port_(port),reusePort_(reusePort),loop_(loop),nextConnId_(0)
{
    threadPool_ = std::unique_ptr<EventloopPool>(new EventloopPool(loop));
}

Server::~Server()
{
    
}

void Server::setThreadNum(int num)
{
    threadPool_->setThreadNum(num);
}

void Server::start()
{
    threadPool_->start();
    loop_ = threadPool_->getNextEvent();
    acceptor_ = std::unique_ptr<Acceptor>(new Acceptor(loop_,port_,reusePort_));
    acceptor_->setNewConnectionCallback(std::bind(&Server::newConnection,this,std::placeholders::_1,std::placeholders::_2));
    acceptor_->listen();
}

void Server::newConnection(int sockfd,sockaddr_in addr)
{
    Eventloop* ioLoop = threadPool_->getNextEvent();
    char buf[64];
    snprintf(buf,sizeof buf,"Connect%d",nextConnId_);
    nextConnId_++;
    string name = buf;
    ConnectionPtr conn(new Connection(ioLoop,name,sockfd,addr));
    connections_[sockfd] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&Server::removeConnection,this,std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&Connection::completeConnect, conn));
}


void Server::removeConnection(const ConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&Server::removeConnectionInLoop,this,conn));
}

void Server::removeConnectionInLoop(const ConnectionPtr& conn)
{
    connections_.erase(conn->getFd());
    Eventloop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&Connection::connectDestroyed, conn));
    if(closeCallback_)
        closeCallback_(conn);
}
