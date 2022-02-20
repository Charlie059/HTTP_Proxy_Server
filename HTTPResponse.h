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
    int maxAge;
    int sMaxAge;
    int content_length;
    string Etag;
    string line;
    string last_modified;
    time_t expire_time;
    __attribute__((unused)) time_t response_time;
    bool no_cache;
    time_t recv_time;
    bool no_store;
    bool is_private;
    bool chunked;
    const string response;

public:
    const string &getEtag() const;
    int getMaxAge() const;
    int getSMaxAge() const;
    const string &getLastModified() const;
    static bool findNotModified(std::string response);
    time_t getExpireTime() const;
    time_t getRecvTime() const;




protected:
    void parseDate();
    void parseMaxAge();
    void parseSMaxAge();
    void parseExpire();
    void parseCache();
    void parseEtag();
    void parsemodify();
    void parseContentLength();
    void setRecvTime();

public:
    //TODO Check the default value if you don't find any filed. eg: if you cannot find Expire, you must not leave a default junk value outside
    HTTPResponse(string response): response(response) {
        this->maxAge = -1;
        this->sMaxAge = -1;
        setRecvTime();
        parseDate();
        parseMaxAge();
        parseSMaxAge();
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
