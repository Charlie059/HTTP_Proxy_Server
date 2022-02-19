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
    void requireHostPort();
    void requireMethod();
public:
    HTTPRequest(string request): raw(request){
        requireMethod();
        requireHostPort();
    }

    string getRaw() const;

    string gethost(){
        return host;
    }
    string getport() {
        return port;
    }
    string getmethod(){
        return method;
    }
};


#endif //TCP_EXAMPLE_REQUEST_H
