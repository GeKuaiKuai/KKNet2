#include "EventHandler.h"
using namespace kknet;

EventHandler::EventHandler(const string& name)
:name_(name),read_(false),write_(false),error_(false),close_(false)
{

}
void EventHandler::setEventFlag(EventType type)
{
    if(type == EventType::kClose)
        close_ = true;
    if(type == EventType::kRead)
        read_ = true;
    if(type == EventType::kWrite)
        write_ = true;
    if(type == EventType::kError)
        error_ = true;

}   


void EventHandler::handleEvents()
{
    if(close_ && closeCallback_)
    {
        closeCallback_();
    }
    if(read_ && readCallback_)
    {
        readCallback_();
    }
    if(write_ && writeCallback_)
    {
        writeCallback_();
    }
    if(error_ && errorCallback_)
    {
        errorCallback_();
    }
    //清空事件标记
    clearEvent();
}