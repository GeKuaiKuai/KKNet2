#ifndef KKNET_TIMESTAMP_H
#define KKNET_TIMESTAMP_H
#include <stdint.h>
#include "public.h"
namespace kknet
{
    class Timestamp
    {
        public:
            Timestamp():microSecondsSinceEpoch_(0)
            {
            }
            explicit Timestamp(int64_t microSecondsSinceEpoch)
            :microSecondsSinceEpoch_(microSecondsSinceEpoch)
            {

            }
            void swap(Timestamp& other)
            {
                std::swap(microSecondsSinceEpoch_,other.microSecondsSinceEpoch_);
            }
            string toString() const;
            string toFormattedString(bool showMicroseconds = true) const;

            bool valid() const
            {
                return microSecondsSinceEpoch_>0;
            }

            int64_t getMicroSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

            time_t getSecondsSinceEpoch() const
            {
                return static_cast<time_t>(microSecondsSinceEpoch_/kMicroSecondsPerSecond);
            }

            
            static Timestamp now();
            static Timestamp invalid()
            {
                return Timestamp(0);
            }

            static Timestamp fromUnixTime(time_t time)
            {
                return fromUnixTIme(time,0);
            }
            static Timestamp fromUnixTIme(time_t time,int microSeconds)
            {
                return Timestamp(static_cast<int64_t>(time)*kMicroSecondsPerSecond+microSeconds);
            }

            static const int kMicroSecondsPerSecond = 1000 * 1000;
        private:
            int64_t microSecondsSinceEpoch_;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs)
    {
        return lhs.getMicroSecondsSinceEpoch() < rhs.getMicroSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs)
    {
    return lhs.getMicroSecondsSinceEpoch() == rhs.getMicroSecondsSinceEpoch();
    }
    
    inline Timestamp addTime(Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.getMicroSecondsSinceEpoch() + delta);
    }
}
#endif