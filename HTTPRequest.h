//
// Created by HP on 2022/2/14.
//
#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#ifndef TCP_EXAMPLE_REQUEST_H
#define TCP_EXAMPLE_REQUEST_H



class HTTPRequest {
private:
    const string raw;

private:
    string host;
    string port;
    string method;
    string line;
    void requireHostPort();
    void requireMethod();
    void requireline();
public:
    HTTPRequest(string request): raw(request){
        requireMethod();
        requireHostPort();
        requireline();
    }

    string getRaw() const;

    string gethost(){
        return host;
    }
    string getline(){
        return line;
    }
    string getport() {
        return port;
    }
    string getmethod(){
        return method;
    }

    static string buildHTTPRequest(string headerLine, bool IfNoneMatch, bool IfModifiedSince, string Etag, string LastModified);
};


#endif //TCP_EXAMPLE_REQUEST_H
