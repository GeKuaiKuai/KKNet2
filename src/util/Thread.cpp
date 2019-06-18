#include "Thread.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
using namespace kknet;

__thread pid_t currentThread::t_id = 0;


void currentThread::cacheTid()
{
    t_id = ::syscall(SYS_gettid);
}



static void* threadFunc(void* data)
{
    ThreadData* threadData = static_cast<ThreadData*>(data);
    threadData->status = ThreadData::Status::kRunning;
    *(threadData->tid) = currentThread::tid();
    threadData->status = ThreadData::Status::kRunning;
    threadData->func();
    delete threadData;
}

int32_t Thread::numCount = 0;

Thread::Thread(NormalFunc func,const string &name)
:func_(func),name_(name),isLoop_(true),threadData_(0),threadId_(0)
{
    if(name_.empty())
    {
        char tmp[32];
        memInit(tmp,sizeof tmp);
        sprintf(tmp,"thread%d",getCount());
    }
}
void Thread::start()
{
    initialiazeThreadData();

    int error = pthread_create(&threadId_,NULL,threadFunc,threadData_);
    //处理错误
    if(error != 0)
    {
        show("线程创建失败");
        destroyThreadData();
    }
    else
    {
        //等待线程执行
        while(threadData_->status != ThreadData::Status::kRunning);
        tid_ = *(threadData_->tid);
    }
}

void Thread::join()
{
    threadData_->status = ThreadData::Status::kJoined;
    pthread_join(threadId_,NULL);
    threadId_ = 0;
    threadData_ = 0;
}

void Thread::stop()
{
    pthread_detach(threadId_);
    threadData_->status = ThreadData::Status::kStop;
    threadData_ = 0;
}

void Thread::initialiazeThreadData()
{
    threadData_ = new ThreadData();
    threadData_->func = func_;
    threadData_->tid = &tid_;
    threadData_->isLoop = isLoop_;
    threadData_->status = ThreadData::Status::kStop;   
}
void Thread::destroyThreadData()
{
    delete threadData_;
    threadData_ = 0;
    threadId_ = 0;
}

Thread::~Thread()
{
    if(threadId_)
    {
        pthread_detach(threadId_);
    }
}



