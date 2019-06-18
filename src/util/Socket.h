#ifndef KKNET_SOCKET_H
#define KKNET_SOCKET_H
#include "public.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LISTEN_QUEUE 1024 
namespace kknet
{
    namespace socket
    {
        int createNonblockSocket(sa_family_t family=AF_INET);
        int createServerSocket(int16_t port,bool loopbackOnly=false);
        void bind(int fd,const string& ip,int16_t port);
        void bind(int fd,int16_t port,bool loopbackOnly);
        void listen(int fd);
        int connect(int fd,const string& ip,int16_t port);
        //聚集读
        ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
        //普通读
        ssize_t read(int sockfd, void *buf, size_t count);
        ssize_t write(int sockfd, const void *buf, size_t count);
        void close(int sockfd);
        void shutdownWrite(int sockfd);
        int accept(int sockfd, struct sockaddr_in* addr);
        //ip转换
        string turnIpToString(const sockaddr_in* addr);
        void turnStringToIp(const string& ip,sockaddr_in* addr);
        void setReuseAddr(int fd,bool on);
        void setKeepAlive(int fd,bool on);

        int getSocketError(int sockfd);
        //字节序转换
        inline uint64_t hostToNetwork64(uint64_t host64)
        {
        return htobe64(host64);
        }

        inline uint32_t hostToNetwork32(uint32_t host32)
        {
        return htobe32(host32);
        }

        inline uint16_t hostToNetwork16(uint16_t host16)
        {
        return htobe16(host16);
        }

        inline uint64_t networkToHost64(uint64_t net64)
        {
        return be64toh(net64);
        }

        inline uint32_t networkToHost32(uint32_t net32)
        {
        return be32toh(net32);
        }

        inline uint16_t networkToHost16(uint16_t net16)
        {
        return be16toh(net16);
        }

    }
}
#endif