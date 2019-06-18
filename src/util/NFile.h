#ifndef KKNET_NFILE_H
#define KKNET_NFILE_H
#include "public.h"
#include <fcntl.h>
#include <unistd.h>
namespace kknet
{
    class NFile
    {
        public:
            NFile()
            :flag_(0)
            {

            }
            ~NFile()
            {
                if(fd_ >= 0)
                    ::close(fd_);
            }
            
            int openRead(const string& url);
            int openWrite(const string& url);
            int openCreate(const string&url,mode_t mode);
            int open(const string& url);
            off_t lseek(off_t off,int whence = SEEK_SET);
            
            size_t read(void* buf,size_t size);
            ssize_t write(const void* data,size_t size);

            void setFlag(int flag,bool add = false)
            {
                if(!add)
                    flag_ = flag;
                else
                    flag_ |= flag; 
                if(fd_)
                    ::fcntl(fd_,F_SETFL,flag_);
            }
            void setCLOEXEC()
            {
                flag_ |= FD_CLOEXEC; 
                if(fd_)
                    ::fcntl(fd_,F_SETFD,FD_CLOEXEC);
            }
            int getFd() const
            {
                return fd_;
            }
            static const int BUFFER_SIZE = 64*1024;

        private:
            int fd_;
            int flag_;
            char buf_[BUFFER_SIZE];

    };
}
#endif