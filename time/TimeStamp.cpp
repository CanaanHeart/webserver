#include "TimeStamp.h"

using namespace std;

TimeStamp TimeStamp::Now()
{
    return TimeStamp();
}

time_t TimeStamp::GetTimeInt() const
{
    return tim_;
}

string TimeStamp::GetTimeStr() const
{
    char buf[20];
    memset(buf, 0, sizeof(buf));
    tm *tm_time = localtime(&tim_);
    snprintf(buf, 20, "%4d-%02d-%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);
    
    return buf;
}