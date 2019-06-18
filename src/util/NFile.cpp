#include "NFile.h"
using namespace kknet;

int NFile::openRead(const string& url)
{
    flag_ |= O_RDONLY|O_CLOEXEC;
    return open(url);
}

int NFile::openWrite(const string& url)
{
    flag_ |= O_WRONLY|O_CLOEXEC;
    return open(url);
}

int NFile::openCreate(const string&url,mode_t mode)
{
    flag_ |= O_CREAT|O_EXCL;
    fd_ = ::open(url.c_str(),flag_,mode);
    if(fd_ < 0)
    {
        show(url+"文件创建失败:");
    }    
}

int NFile::open(const string& url)
{
    assert(flag_ != 0);
    fd_ = ::open(url.c_str(),flag_);
    if(fd_ < 0)
    {
        show(url+"文件打开失败:");
    }

}

off_t NFile::lseek(off_t off,int whence)
{
    return ::lseek(fd_,off,whence);
}

size_t NFile::read(void* buf,size_t size)
{
    size_t s = ::read(fd_,buf,size);
    return s;
}

ssize_t NFile::write(const void* data,size_t size)
{
    ssize_t s = ::write(fd_,data,size);
    return s;
}