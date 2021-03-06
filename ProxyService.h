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
#include "Request.h"
#include "Client.h"


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
    [[noreturn]] static void run();

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


    static string recvRequest(const void *req);

    static int verifyRequest(const void *req, HTTPRequest &httpRequest);

    static int verifyHostServer(const void *req, const Client &client);

    static int handleConnect(void *req, HTTPRequest request, Client client);

    static int handleGet(void *req, HTTPRequest & request, Client client);

    static string recvResponse(const Client &client);

    static void handleChunked(const void *req, const Client &client, string &server_msg);

    static string recvAllResponse(Client client, string server_meg, int contentLength);

    static bool revalidate(HTTPRequest &httpRequest, const HTTPResponse &parse, Client & client, void * req);

    static bool isFresh(time_t recv_time, const HTTPResponse &parse);

    static void sendCache(const void *req, vector<char> &response_vec, HTTPResponse & response);
};


#endif //HTTP_CACHING_PROXY_PROXYSERVICE_H
