#ifndef KKNET_TIMEWHEEL_H
#define KKNET_TIMEWHEEL_H
#include "../util/typedef.h"
#include "Connection.h"
#include <set>
namespace kknet
{
    struct TimeWheelEntry
    {
        explicit TimeWheelEntry(const WeakConnectionPtr ptr):weakPtr_(ptr)
        {
        }
        ~TimeWheelEntry()
        {
            ConnectionPtr conn = weakPtr_.lock();
            if(conn)
                conn->shutdown();
        }
        WeakConnectionPtr weakPtr_;
    };
    typedef std::shared_ptr<TimeWheelEntry> EntryPtr;
    typedef std::set<EntryPtr> WheelSet;

    struct WheelNode
    {
        WheelNode():next_(0),set_(new WheelSet()){}
        ~WheelNode()
        {
            delete set_;
        }
        WheelNode* next_;
        WheelSet* set_;
    };

    class TimeWheel
    {
        public:
            //wheel个数，单位是秒，如需8秒关闭连接就设置为8
            void setMaxNum(int num)
            {
                maxNum_ = num;
                head_ = new WheelNode();
                WheelNode* walk = head_;
                for(int n=1;n<num;n++)
                {
                    WheelNode* next = new WheelNode();
                    walk->next_ = next;
                    walk = next;
                }
                tail_ = walk;
                tail_->next_ = head_;
            }
            void push(EntryPtr ptr)
            {
                currentNode_->set_->insert(ptr);
            }
            void onTime()
            {
                currentNode_ = currentNode_->next_;
                if(!currentNode_->set_->empty())
                {
                    delete currentNode_->set_;
                    currentNode_->set_ = new WheelSet();
                }
            }
            ~TimeWheel()
            {
                WheelNode* walk = head_;
                while(walk)
                {
                    WheelNode* temp = walk;
                    walk = walk->next_;
                    delete temp;
                }
            }
        private:
            //时间轮最大数目
            int maxNum_;
            WheelNode* nodeslist_;
            WheelNode* currentNode_;
            WheelNode* head_;
            WheelNode* tail_;
    };
}
#endif