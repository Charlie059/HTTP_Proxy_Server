//
// Created by X G on 2/14/22.
//

#ifndef HTTP_CACHING_PROXY_REQUEST_H
#define HTTP_CACHING_PROXY_REQUEST_H

#include <string>
#include <vector>

static int nextID = 0;
class Request {

private:
    int requestID; // One request has a unique id
    int fd; // record the client fd
    std::string ip_address; // record the client ip_address;

public:
    /**
     * Constructor of the Request
     * @param fd client's fd
     * @param ip_address client ip address
     */
    Request(int fd, std::string ip_address): fd(fd), ip_address(ip_address){
        this->requestID = nextID;
        nextID++;
    }

    /**
     * Get the id of the request
     * @return id
     */
    int getRequestId() const;

    /**
     * Get the client's fd
     * @return client's fd
     */
    int getFd() const;

    /**
     * Get the client's ip address
     * @return client's ip address string
     */
    const std::string &getIpAddress() const;
};


#endif //HTTP_CACHING_PROXY_REQUEST_H

