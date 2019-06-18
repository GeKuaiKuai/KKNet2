#include "Socket.h"
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
using namespace kknet;
int socket::createNonblockSocket(sa_family_t family)
{
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd == -1)
        show("socket创建失败");
    return sockfd;
}

int socket::createServerSocket(int16_t port,bool loopbackOnly)
{
    int fd = socket::createNonblockSocket();
    if(fd != -1)
    {
        socket::bind(fd,port,loopbackOnly);
    }
    return fd;

}

void socket::bind(int fd,int16_t port,bool loopbackOnly)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_ANY : INADDR_LOOPBACK;
    addr.sin_addr.s_addr = socket::hostToNetwork32(ip);
    addr.sin_port = socket::hostToNetwork16(port);
    int ret = ::bind(fd,static_cast<sockaddr*>(static_cast<void*>(&addr)),sizeof addr);
    if(ret < 0)
        show("bind失败");

}
void socket::bind(int fd,const string& ip,int16_t port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = socket::hostToNetwork16(port);
    socket::turnStringToIp(ip,&addr);
    int ret = ::bind(fd,static_cast<sockaddr*>(static_cast<void*>(&addr)),sizeof addr);
    if(ret < 0)
        show("bind失败");    
    
}

string socket::turnIpToString(const sockaddr_in* addr)
{
    char buf[20];
    memInit(buf,sizeof buf);
    ::inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(20));
    return buf;
}

void socket::turnStringToIp(const string& ip,sockaddr_in* addr)
{
    if(::inet_pton(AF_INET, ip.c_str(), &addr->sin_addr) <= 0)
        show("ip转换失败");
}

int socket::connect(int fd,const string& ip,int16_t port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = socket::hostToNetwork16(port);
    socket::turnStringToIp(ip,&addr);
    return ::connect(fd, static_cast<sockaddr*>(static_cast<void*>(&addr)), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}


ssize_t socket::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
  return ::readv(sockfd, iov, iovcnt);
}

void socket::listen(int fd)
{
    ::listen(fd,MAX_LISTEN_QUEUE);
}

ssize_t socket::read(int sockfd, void *buf, size_t count)
{
  return ::read(sockfd, buf, count);
}

ssize_t socket::write(int sockfd, const void *buf, size_t count)
{
  return ::write(sockfd, buf, count);
}

void socket::close(int sockfd)
{
  ::close(sockfd);
}

void socket::shutdownWrite(int sockfd)
{
    ::shutdown(sockfd, SHUT_WR);
}

int socket::accept(int sockfd, struct sockaddr_in* addr)
{
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  int connfd = ::accept4(sockfd,static_cast<sockaddr*>(static_cast<void*>(addr)),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0)
  {
    int savedErrno = errno;
    switch (savedErrno)
    {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        break;
      default:
        break;
    }
  }
  return connfd;
}
void socket::setReuseAddr(int fd,bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                &optval, static_cast<socklen_t>(sizeof optval));    
}
void socket::setKeepAlive(int fd,bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                &optval, static_cast<socklen_t>(sizeof optval));  
}

int socket::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }

}
