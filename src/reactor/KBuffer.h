#ifndef KKNET_KBUFFER_H
#define KKNET_KBUFFER_H
/* 
    name:非连续应用层缓冲区
    author:个快快
    version:0.1
*/
#include <cstring>
#include "../util/typedef.h"
#include "../util/public.h"
#define BLOCK_SIZE 1024
#define MAX_BLOCK_SIZE 1310720
namespace kknet
{
    struct DataBlock
    {
        public:
            DataBlock():last(0),next(0),buf{0}
            {

            }
            char buf[BLOCK_SIZE];
            DataBlock *last;
            DataBlock *next;
    };
    class KBuffer
    {
        public:
            KBuffer();

            void showMessage()
            {
                printf("usedSize:%u\nreadable:%u\nwriteble:%u\nbuflen:%u\n",usedSize_,readableSize_,writableSize_,buflen_);
                int n = 1;
                DataBlock *walk = head_;
                while(walk!=tail_)
                {
                    walk = walk->next;
                    n++;
                }
                printf("当前总块数:%d\n",n);
            }
            
            ~KBuffer();

            /*
            * 读取并前进可读指针
            */
            void read(void* buf,size_t size)
            {
                peek(buf,size);
                retrieve(size);
            }

            /*
            * 只读取，不前进
            */
            void peek(void* buf,size_t size);
            /*
            * 测试使用，读取全部数据，返回string
            */
            string peekAllAsString()
            {
                char* buf = new char[buflen_]();
                peek(buf,readableSize_);
                string res(buf);
                delete[] buf;
                return res;                
            }
            string readAllAsString()
            {
                char* buf = new char[buflen_]();
                read(buf,readableSize_);
                string res(buf);
                delete[] buf;
                return res;
            }
            /*
            *  返回成功写入字节数，返回0表示缓冲区已满或者写入数据过大
            */
            size_t write(const void* buf,size_t size);


            size_t getReadableSize() const
            {
                return readableSize_;
            }

            size_t getWritableSize() const
            {
                return writableSize_;
            }
            
            void retrieve(size_t len);

            /*
            * 读取文件描述符内容
            */
            ssize_t readFd(int fd);

            bool getLine(char* buf);


        private:

            void do_write(const void* buf,size_t size);

            /*
            *   将已经被读取的block移动至尾部
            */
            void move_head();
            /*
            *   扩充空间
            */
            void makeSpace(size_t len);
            
            DataBlock *head_;
            DataBlock *tail_;
            //可读位置所在块指针
            DataBlock *readstr_;
            //可写位置所在块指针
            DataBlock *writestr_;
            //可读位置在所在block的偏移
            size_t readablePos_;
            //可写位置在所在block的偏移
            size_t writablePos_;
            //从可读到可写位置的size
            size_t readableSize_;
            //从可写到最后一个block末尾的size
            size_t writableSize_;
            //从头部到可读位置的size
            size_t usedSize_;
            //当前缓冲区总大小
            size_t buflen_;


            
    };
}
#endif