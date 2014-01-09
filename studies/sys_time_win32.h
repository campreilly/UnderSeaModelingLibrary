/** 
 * @file sys_time_win32.h
 * This file provides Linux stuctures timezone, and 
 * the gettimeofday function for compatibility with WIN32
 */

#ifdef WIN32

#include <time.h>
#include <windows.h>
#include <winsock.h>

const __int64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;

struct timezone 
{
  __int32  tz_minuteswest; /* minutes W of Greenwich */
  bool  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv/*in*/, struct timezone *tz/*in*/)
{
  FILETIME ft;
  __int64 tmpres = 0;
  TIME_ZONE_INFORMATION tz_winapi;
  int rez=0;

   ZeroMemory(&ft,sizeof(ft));
   ZeroMemory(&tz_winapi,sizeof(tz_winapi));

    GetSystemTimeAsFileTime(&ft);

    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tv->tv_sec = (__int32)(tmpres*0.000001);
    tv->tv_usec =(tmpres%1000000);


    //_tzset(),don't work properly, so we use GetTimeZoneInformation
    rez=GetTimeZoneInformation(&tz_winapi);
    tz->tz_dsttime=(rez==2)?true:false;
    tz->tz_minuteswest = tz_winapi.Bias + ((rez==2)?tz_winapi.DaylightBias:0);

  return 0;
}
#endif