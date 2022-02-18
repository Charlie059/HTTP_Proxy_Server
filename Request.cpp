//
// Created by X G on 2/14/22.
//

#include <string>
#include "Request.h"

//#define DEBUG_MODE
#ifdef DEBUG_MODE
#include "gtest/gtest.h"
#endif

int Request::getRequestId() const {
    return requestID;
}

int Request::getFd() const {
    return fd;
}

const std::string & Request::getIpAddress() const {
    return ip_address;
}
