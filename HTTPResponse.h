//
// Created by HP on 2022/2/14.
//
#ifndef HOMEWORK568_HTTPRESPONSE_H
#define HOMEWORK568_HTTPRESPONSE_H

//#define DEBUG_MODE
#ifdef DEBUG_MODE
#include "gtest/gtest.h"
#endif

//#include <fcntl.h>

#include <time.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include "algorithm"

using namespace std;

class HTTPResponse{
private:
    string response;
public:
    string Etag;
    string line;
    int max;
    string last_modified;
    time_t expire_time;
    time_t response_time;
    bool no_cache;
    bool chunked;

    bool isChunked() const;


    HTTPResponse(string response): response(response) {
        parseDate();
        parseMax();
        parseExpire();
        parseCache();
        parseEtag();
        parsemodify();
        isChunk();
    }
    void parseDate();
    void parseMax();
    void parseExpire();
    void parseCache();
    void parseEtag();
    void parsemodify();
    void isChunk() {
        size_t pos;
        if ((pos = this->response.find("chunked")) != std::string::npos) {
            this->chunked = true;
            return;
        }
        else this->chunked = false;
    }


};


#endif //HOMEWORK568_HTTPRESPONSE_H
