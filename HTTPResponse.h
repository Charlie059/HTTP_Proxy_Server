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

// TODO not every thing is public
using namespace std;

class HTTPResponse{
private:
    string Etag;
    string line;
public:
    const string &getLine() const;
    bool isChunked() const;
private:
    int max;
    int content_length;
public:
    int getContentLength() const;

private:
    string last_modified;
    time_t expire_time;
    time_t response_time;
    bool no_cache;
    bool chunked;
    const string response;

protected:
    void parseDate();
    void parseMax();
    void parseExpire();
    void parseCache();
    void parseEtag();
    void parsemodify();
    void parseContentLength();

public:
    HTTPResponse(string response): response(response) {
        parseDate();
        parseMax();
        parseExpire();
        parseCache();
        parseEtag();
        parsemodify();
        isChunk();
    }


    // TODO send to the .cpp file
    void isChunk() {
        size_t pos;
        if ((pos = this->response.find("chunked")) != std::string::npos) {
            this->chunked = true;
            return;
        }
        else this->chunked = false;
    }


    /**
     * Build the HTTP Response, eg 400 BAD REQUEST
     * @return string for messgae
     */
    static string buildResponse(int HTTPCODE){
        switch (HTTPCODE) {
            case 200:
                return "HTTP/1.1 200 OK\n"
                       "Content-Type: application/json; charset=UTF-8\n"
                       "Content-Length: 0\n"
                       "\r\n";
            case 400:
                return "HTTP/1.1 400 Bad Request\n"
                       "Content-Type: application/json; charset=UTF-8\n"
                       "Content-Length: 0\n"
                       "\r\n";
            default:
                return "HTTP/1.1 400 Bad Request\n"
                       "Content-Type: application/json; charset=UTF-8\n"
                       "Content-Length: 0\n"
                       "\r\n";
        }
    }


};


#endif //HOMEWORK568_HTTPRESPONSE_H
