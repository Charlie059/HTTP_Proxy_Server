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
#include <vector>
#include "algorithm"

// TODO not every thing is public
using namespace std;

class HTTPResponse{
public:
    const vector<char> &getResponseRawData() const;
    void setResponseRawData(const vector<char> &responseRawData);
    int getContentLength() const;
    const string &getLine() const;
    bool isChunked() const;
    bool isNoCache() const;
    bool isNoStore() const;
    bool isPrivate() const;

/**
 * if response find no-store and private return false else return true
 * @return if can be store in the cache
 */
    bool isCacheable();

private:
    int max;
    int content_length;
    string Etag;
    string line;
    string last_modified;
    time_t expire_time;
    time_t response_time;
    bool no_cache;
    bool chunked;
    const string response;
    std::vector<char> response_raw_data;
    void isChunk();

protected:
    void parseDate();
    void parseMax();
    void parseExpire();
    void parseCache();
    void parseEtag();
    void parsemodify();
    void parseContentLength();
    void setRecvTime();

public:
    HTTPResponse(string response): response(response), response_raw_data({}) {
        setRecvTime();
        parseDate();
        parseMax();
        parseExpire();
        parseCache();
        parseEtag();
        parsemodify();
        parseContentLength();
        isChunk();
    }






    /**
     * Build the HTTP Response, eg 400 BAD REQUEST
     * @return string for messgae
     */
    static string buildResponse(int HTTPCODE);

};


#endif //HOMEWORK568_HTTPRESPONSE_H
