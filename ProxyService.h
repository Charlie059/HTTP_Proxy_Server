//
// Created by X G on 2/14/22.
//

#ifndef HTTP_CACHING_PROXY_PROXYSERVICE_H
#define HTTP_CACHING_PROXY_PROXYSERVICE_H
#define MAX_TCP_LEN 65535
#define PORTNUM 12345


#include <ctime>
#include <fstream>
#include <unordered_map>
#include <sstream>

#include "Server.h"
#include "ProxyService.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"


class ProxyService {
private:
    const int portNum;


public:
    /**
     * Call a ProxyService object with portNum
     * @param portNum PORTNUM 12345
     */
    explicit ProxyService(const int portNum): portNum(portNum){};

    /**
     * Run the Demon
     */
    [[noreturn]] void run();

    /**
     * Write log with thread safe
     * @param logTo String
     */
    static void writeLog(const std::string& logTo);

    /**
     * Handle the request by each thread
     * @param request object
     * @return nullptr to end the thread //TODO CHECK
     */
    static void *handle(void * request);



};


#endif //HTTP_CACHING_PROXY_PROXYSERVICE_H
