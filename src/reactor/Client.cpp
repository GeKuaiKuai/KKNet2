#include "Client.h"
#include "ClientConnector.h"
#include "Eventloop.h"
#include "Connection.h"

using namespace kknet;

void kknet::removeConnection(Eventloop* loop,const ConnectionPtr& conn)
{
    loop->queueInLoop(std::bind(&Connection::connectDestroyed, conn));
}

Client::Client(Eventloop* loop,const string& ip,int16_t port,const string& nameArg)
:loop_(loop),connector_(new ClientConnector(loop,ip,port)),
name_(nameArg),connectionCallback_(defaultConnectionCallback),messageCallback_(defaultReadCallback),
retry_(false),connect_(true),nextConnId_(1),ip_(ip),port_(port)
{
  connector_->setNewConnectionCallback(
      std::bind(&Client::newConnection, this, std::placeholders::_1));
}

Client::~Client()
{
    ConnectionPtr conn;
    bool unique = false;
    {
        mutex_.lock();
        unique = connection_.unique();
        conn = connection_;
        mutex_.unlock();
    }
    if(conn)
    {

        if (unique)
        {
            conn->forceClose();
        }
    }
}

void Client::setRetryCallback(NormalFunc cb)
{
    connector_->setRetryCallback(cb);
}

void Client::connect()
{
    connect_ = true;
    connector_->start();
}

void Client::disconnect()
{
    connect_ = false;
    {
        mutex_.lock();
        if(connection_)
        {
            connection_->shutdown();
        }
        mutex_.unlock();
    }
}

void Client::stop()
{
    connect_ = false;
    connector_->stop();
}

void Client::newConnection(int sockfd)
{
    char buf[32];
    snprintf(buf, sizeof buf, "连接%d",nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;
    sockaddr_in addr;
    string ip = connector_->getIp();
    int16_t port = connector_->getPort();
    socket::turnStringToIp(ip,&addr);
    addr.sin_port = port;

    ConnectionPtr conn(new Connection(loop_,
                                          connName,
                                          sockfd,
                                          addr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    {
        mutex_.lock();
        connection_ = conn;
        mutex_.unlock();
    }
    conn->completeConnect();

}

void Client::removeConnection(const ConnectionPtr& conn)
{
    {
        mutex_.lock();
        connection_.reset();
        mutex_.unlock();
    }

    loop_->queueInLoop(std::bind(&Connection::connectDestroyed, conn));
    
    if (retry_ && connect_)
    {
        connector_->restart();
    }    


}

bool Client::isConnected() const
{
    return connector_->isConnected();
}
