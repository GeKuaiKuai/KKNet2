#ifndef KKNET_THREAD_H
#define KKNET_THREAD_H
#include "public.h"
#include "typedef.h"


namespace kknet
{
    //线程数据
    struct ThreadData
    {
        enum Status
        {
            kStop,
            kRunning,
            kJoined,
        };

        NormalFunc func;
        pid_t *tid;
        Status status;
        bool isLoop;

    };

    class Thread
    {
        public:
            Thread(NormalFunc func,const string &name="");
            ~Thread();
            //开始执行线程，此时真正通过系统调用创建线程
            void start();
            //销毁对象后线程也会自动停止
            void stop();
            void join();
            void setName(const string& name)
            {
                this->name_ = name;
            }
            void setLoop(bool loop)
            {
                isLoop_ = loop;
            }
            const string& getName()
            {
                return name_;
            }
            static int32_t getCount()
            {
                
                return ++numCount;
            }

        private:

            void initialiazeThreadData();
            void destroyThreadData();
            NormalFunc func_;
            string name_;
            bool isLoop_;
            pid_t tid_;
            pthread_t threadId_;
            //线程数据，需要线程自己释放
            ThreadData* threadData_;
            //线程计数
            static int32_t numCount;

            
    };


    
    namespace currentThread
    {
        //获取当前线程tid
        extern __thread pid_t t_id;
        void cacheTid();
        inline int tid()
        {
            if(t_id == 0)
            {
                cacheTid();
            }
            return t_id;
        }
    }

}

#endif