//
// Created by X G on 2/17/22.
//

#ifndef HTTP_PROXY_SERVER_TIME_H
#define HTTP_PROXY_SERVER_TIME_H
#include <iostream>
#include <ctime>
using namespace std;

class Time {
public:
    Time(){};

static string getCurrentTime(){
    time_t now = time(0);
    tm *tm_gmt=gmtime(&now);
    char * curr=asctime(tm_gmt);
    string currentTime(curr);
    return currentTime;
}

    static time_t getCurrentTm(){
        time_t now = time(0);
        tm *tm_gmt=gmtime(&now);
        time_t time2=mktime(tm_gmt);
        return time2;
    }


};


#endif //HTTP_PROXY_SERVER_TIME_H
