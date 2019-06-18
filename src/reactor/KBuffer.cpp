#include "KBuffer.h"
#include "../util/Socket.h"
#include "stdio.h"
using namespace kknet;

KBuffer::KBuffer():
head_(0),tail_(0),readstr_(0),writestr_(0),readablePos_(0),writablePos_(0),readableSize_(0),
writableSize_(0),usedSize_(0),buflen_(0)
{
   
}

KBuffer::~KBuffer()
{
    DataBlock* walk = head_;
    DataBlock* temp = walk;
    while(walk)
    {
        temp = walk;
        walk = walk->next;
        delete temp;
    }
}

ssize_t KBuffer::readFd(int fd)
{
    char extrabuf[65536];
    struct iovec vec[MAX_BLOCK_SIZE/BLOCK_SIZE+1];
    int n = 0;
    DataBlock* walk = writestr_;
    while(walk)
    {
        if(walk == writestr_)
        {
            vec[n].iov_base = walk->buf+writablePos_;
            vec[n].iov_len = BLOCK_SIZE - writablePos_;
        }
        else
        {
            vec[n].iov_base = walk->buf;
            vec[n].iov_len = BLOCK_SIZE;
        }
        ++n;
        walk = walk->next;
    }

    size_t writable = getWritableSize();

    vec[n].iov_base = extrabuf;
    vec[n].iov_len = sizeof extrabuf;
    ++n;
    
    ssize_t t = socket::readv(fd,vec,n);
    
    if(t < 0)
    {
        show("readv错误");
        printf(" errno:%d fd:%d n:%d\n",errno,fd,n);
        
    }
    else if(static_cast<size_t>(t) <= writable)
    {
        
        writableSize_ -= t;
        readableSize_ += t;
        if(t <= BLOCK_SIZE - writablePos_)
        {
            writablePos_ += t;
        }
        else
        {
            int pos = (t - (BLOCK_SIZE - writablePos_))/BLOCK_SIZE;
            writablePos_ = (t - (BLOCK_SIZE - writablePos_)) % BLOCK_SIZE;
            if(writablePos_ == 0)
            {
                writablePos_ = BLOCK_SIZE;
            }
            else
            {
                ++pos;
            }

            writestr_ = static_cast<DataBlock*>(vec[pos].iov_base);
                      
        }
        return t;
        
    }
    else
    {
        writableSize_ = 0;
        writestr_ = tail_;
        if(tail_)
            writablePos_ = BLOCK_SIZE;
        else
            writablePos_ = 0;
        readableSize_ += writable;
        size_t t2 = write(extrabuf,t - writable);
        return (writable+t2);
    }
    

}

void KBuffer::peek(void* buf,size_t size)
{
    size_t l = size > readableSize_ ? readableSize_ : size;
    size_t n = 0;
    DataBlock* walk = readstr_;
    while(l)
    {
        if(walk == readstr_)
        {
            if(BLOCK_SIZE - readablePos_ >= l)
            {
                std::memcpy(static_cast<char*>(buf)+n,walk->buf+readablePos_,l);
                l = 0;
                break;
            }
            else
            {
                std::memcpy(static_cast<char*>(buf)+n,walk->buf+readablePos_,BLOCK_SIZE - readablePos_);
                n += (BLOCK_SIZE - readablePos_);
                l -= (BLOCK_SIZE - readablePos_);
            }
        }
        else
        {
            if(BLOCK_SIZE >= l)
            {
                std::memcpy(static_cast<char*>(buf)+n,walk->buf,l);
                l = 0;
                break;
            }
            else
            {
                std::memcpy(static_cast<char*>(buf)+n,walk->buf,BLOCK_SIZE);
                l -= BLOCK_SIZE;
                n += BLOCK_SIZE;
            }
        }

        walk = walk->next;
    }
}

size_t KBuffer::write(const void* buf,size_t size)
{
    if(size > usedSize_ - readablePos_ + writableSize_ && buflen_ >= MAX_BLOCK_SIZE)
    {
        return 0;
    }
    
    if(size <= writableSize_)
    {
        do_write(buf,size);
        return size;
    }
    else if(size <= usedSize_ - readablePos_ + writableSize_ )
    {
        move_head();
        do_write(buf,size);
        return size;
    }
    else
    {
        move_head();
        if(size > MAX_BLOCK_SIZE - writableSize_)
        {
            return 0;
        }
        else
        {
            makeSpace(size);
            do_write(buf,size);
            return size;
        }
    }


}

 void KBuffer::do_write(const void* buf,size_t size)
{
    size_t l = 0;
    DataBlock *walk = writestr_;
    while(l < size)
    {
        if(walk == writestr_)
        {
            if((size - l)<= BLOCK_SIZE - writablePos_)
            {
                ::memcpy(walk->buf+writablePos_,static_cast<const char*>(buf)+l,size);
                l = size;
                writablePos_ += l;
                break;
            }
            else
            {
                ::memcpy(walk->buf+writablePos_,static_cast<const char*>(buf)+l,BLOCK_SIZE - writablePos_);
                l += (BLOCK_SIZE - writablePos_);
            }
        }
        else
        {
            if((size - l) <= BLOCK_SIZE)
            {
                ::memcpy(walk->buf,static_cast<const char*>(buf)+l,size - l);
                writablePos_ = (size - l);
                l = size;
                break;
            }
            else
            {
                ::memcpy(walk->buf,static_cast<const char*>(buf)+l,BLOCK_SIZE);
                l += BLOCK_SIZE;

            }
        }
        walk = walk->next;
    }
    readableSize_ += size;
    writableSize_ -= size;
    if(writablePos_ == BLOCK_SIZE && walk->next)
    {
       writestr_ = walk->next;
       writablePos_ = 0;
    }
    else
        writestr_ = walk;
}

void KBuffer::move_head()
{
    if(readstr_ != head_)
    {
        DataBlock* temp = readstr_->last;
        readstr_->last->next = 0;
        readstr_->last = 0;
        head_->last = tail_;
        tail_->next = head_;
        tail_ = temp;
        head_ = readstr_;
        writableSize_ += (usedSize_ - readablePos_);
        usedSize_ = readablePos_;
    }

}

bool KBuffer::getLine(char *buf)
{          
    int i = 0;
    char ch = 0; 
    DataBlock *tmp_readstr = readstr_;
    size_t tmp_readablePos = readablePos_;

    if(tmp_readablePos == BLOCK_SIZE)
    {
        tmp_readablePos = 0;
        tmp_readstr = tmp_readstr->next;
    }
    
    ch = tmp_readstr->buf[tmp_readablePos];
    while(ch != '\n')
    {
        ++i;
        ++tmp_readablePos;
        if(i>=readableSize_)
        {
            return false;
        }
        if(tmp_readablePos == BLOCK_SIZE)
        {
            tmp_readablePos = 0;
            tmp_readstr = tmp_readstr->next;
        }
        ch = tmp_readstr->buf[tmp_readablePos];
    }
    
    read(buf,i+1);
    return true;           
}

void KBuffer::makeSpace(size_t len)
{
    DataBlock* walk = tail_;
    while(len)
    {
        DataBlock *k = new DataBlock();
        k->last = walk;
        if(walk)
            walk->next = k;
        else
        {
            head_ = k;
            writestr_ = k;
            readstr_ = k;
        }
            
        walk = k;
        if(len > BLOCK_SIZE)
            len -= BLOCK_SIZE;
        else
            len = 0;
        buflen_ += BLOCK_SIZE;
        writableSize_ += BLOCK_SIZE;
    }
    tail_ = walk;
    if(writablePos_ == BLOCK_SIZE)
    {
        writestr_ = writestr_->next;
        writablePos_ = 0;
    }
}
void KBuffer::retrieve(size_t len)
{
    size_t l = len = len > readableSize_? readableSize_ : len;
    readableSize_ -= l;
    usedSize_ += l;
    DataBlock *walk = readstr_;
    while(len)
    {
        if(walk == readstr_)
        {
            if(BLOCK_SIZE - readablePos_ >= l)
            {
                readablePos_ += l;
                l = 0;
                break;
            }
            else
            {
                l -= (BLOCK_SIZE - readablePos_);
            }
        }
        else
        {
            if(BLOCK_SIZE >= l)
            {
                readablePos_ = l;
                l = 0;
                break;                    
            }
            else
            {
                l -= BLOCK_SIZE;
            }
        }
        walk = walk->next;
    }
    if(readablePos_ == BLOCK_SIZE && walk->next)
    {
        readstr_ = walk->next;
        readablePos_ = 0;
    }
    else
        readstr_ = walk;

}