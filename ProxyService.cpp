//
// Created by X G on 2/14/22.
//

#include <sstream>
#include "ProxyService.h"
#include "Server.h"
#include "Client.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "Request.h"
#include <pthread.h>

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

std::ofstream logFile("proxy.log"); // TODO change back

[[noreturn]] void ProxyService::run() {
    // Build Server and let it listen port eg: 12345
    Server server(PORTNUM);
    // Error handling
    if(server.tryListen() == -1){
        writeLog("NOTE: Error to listen on the port");
        exit(EXIT_FAILURE);
    }
    // Build Demon
    while(true){
        // Accept one connection in the queue
        if(server.tryAccept() == -1){
            writeLog("NOTE: Error to accept on the port");
            continue;
        }
        // Create a request
        Request * request = new Request(server.getBrowserFd(), server.getCurrBrowserIp());
        // Create a new thread to handle the request
        pthread_t thread;
        pthread_create(&thread, NULL, handle, request);
    }
}

/**
 * Handling to request form the client
 * @param info
 * @return
 */
void * ProxyService::handle(void * req) {
    // Extract the request

    // Receive the request info from client

     // Error handling
    // 1. if request info's length <= 0 -> log and return

    // 2. if request is empty or \r or \n or \r\n -> return


    // Parse the request info

    // log the request

    // Error handling
    // if not CONNECT or POST or GET -> log and return 400 Bad request


    // if parse success, log info

    // Connect to the Server
    // Build Client to connect server

    // Error handling


    // end

    return nullptr;

}

/**
 * Write log by lock
 * @param logTo String
 */
void ProxyService::writeLog(const std::string& logTo){
    pthread_mutex_lock(&mutex_lock);
    logFile << logTo << std::endl;
    pthread_mutex_unlock(&mutex_lock);
}



