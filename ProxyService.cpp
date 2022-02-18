//
// Created by X G on 2/14/22.
//

#include "ProxyService.h"
#include "Client.h"
#include "Time.h"

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile("proxy.log"); // TODO change back

[[noreturn]] void ProxyService::run() {
    // Build Server and let it listen port eg: 12345
    Server server(PORTNUM);
    // Error handling
    if(server.getErrorCode() == -1 || server.tryListen() == -1){
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
        Request * request = new Request(server.getClientFd(), server.getClientIp()); // TODO free
        // Create a new thread to handle the request
        pthread_t thread;
        pthread_create(&thread, nullptr, handle, request);
    }
}

/**
 * Handling to request form the client
 * @param req request object pointer
 * @return
 */
void * ProxyService::handle(void * req) {
    // Receive the request info from client
    string requestContent;
    if((requestContent = recvRequest(req)).empty() || requestContent == "\r" || requestContent == "\n" || requestContent == "\r\n") return nullptr;

    // Parse the request info
    HTTPRequest httpRequest(requestContent);

    // verifyRequest TODO return 400 Bad request
    if(verifyRequest(req, httpRequest) == -1) return nullptr;

    // Connect to the Server
    Client client(httpRequest.gethost().c_str(), stoi(httpRequest.getport()));
    if(verifyHostServer(req, client) == -1) return nullptr;

    // If current client request HEAD is CONNECT
    if(httpRequest.getmethod() == "CONNECT"){
        // do CONNECT stuff
        handleConnect(req, httpRequest, client);
    }
    else{ // else if POST or GET
        // do POST or GET stuff
        if(handleGet(req, httpRequest, client) == -1){
            //TODO log ERROR
            close(((Request*)req)->getFd()); // close client connection
            client.close();
            return nullptr;
        }
    }

    // Close Connection
    close(((Request*)req)->getFd()); // close client connection
    client.close();
    return nullptr;
}

/**
 * Verify the Host server
 * @param req object
 * @param client object
 * @return -1 for error, 0 for ok
 */
int ProxyService::verifyHostServer(const void *req, const Client &client) {
    if(client.getErrorCode() == -1){
        // log error
        writeLog("Invalid Request HOST: cannot get address info for host");
        // Send the 400 code
        Server::trySendMessage(const_cast<char *>(HTTPResponse::buildResponse(400).c_str()), ((Request*)req)->getFd());
        close(((Request*)req)->getFd()); // close client side connection
        client.close(); // close server side connection
        return -1;
    }
    return 0;
}

// TODO may in the new class -> checker
/**
 * Verify the HTTPRequest
 * @param req the request object ptr
 * @param httpRequest HTTPRequest object
 * @return if -1 error, else 0 for ok
 */
int ProxyService::verifyRequest(const void *req, HTTPRequest &httpRequest) {
    string logInfo;
    if(httpRequest.getmethod() != "CONNECT" && httpRequest.getmethod() != "POST" && httpRequest.getmethod() != "GET"){
        logInfo = to_string(((Request*)req)->getRequestId()) + ": Invalid Request Method";
    } else if(httpRequest.gethost().empty()){
        logInfo = to_string(((Request*)req)->getRequestId()) + ": Invalid Request HOST";
    } else{
        logInfo = to_string(((Request*)req)->getRequestId()) + ": \"" + httpRequest.gethost() + "\" from " + ((Request*)req)->getIpAddress() + " @ " + Time::getCurrentTime();
        writeLog(logInfo);
        return 0;
    }
    // Send the 400 code
    Server::trySendMessage(const_cast<char *>(HTTPResponse::buildResponse(400).c_str()), ((Request*)req)->getFd());
    writeLog(logInfo);
    close(((Request*)req)->getFd()); // close client connection
    return -1;
}

/**
 * Receive the HTTP Request
 * @param req request ptr
 * @return String for result of request
 */
string ProxyService::recvRequest(const void *req) {
    char HTTPRequest[MAX_TCP_LEN];
    int res = Server::tryRecvMessage(HTTPRequest, 0, ((Request*)req)->getFd());
    string logInfo;
    switch (res) {
        case -1: // if return an error
            logInfo = to_string(((Request*)req)->getRequestId()) + ": " + "Error to revive the HTTP Request from " + ((Request*)req)->getIpAddress();
            writeLog(logInfo);
            close(((Request*)req)->getFd()); // close client connection
            return "";
        case 0: // if client close the TCP connection
            logInfo = to_string(((Request*)req)->getRequestId()) + ": " + "Error to revive the HTTP Request from " + ((Request*)req)->getIpAddress() + " because client close the connection";
            writeLog(logInfo);
            close(((Request*)req)->getFd()); // close client connection
            return "";
        default:
            return string(HTTPRequest, res);
    }
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


/**
 * Handle Connect TODO
 * @param req
 * @param request
 * @param client
 * @return
 */
int ProxyService::handleConnect(void *req, HTTPRequest request, Client client) {

    return 0;
}

/**
 * Handle get or post method
 * @param req
 * @param request
 * @param client
 * @return
 */
int ProxyService::handleGet(void *req, HTTPRequest & request, Client client) {
    // send message to the server
    if(Client::trySendMessage(const_cast<char *>(request.getRaw().c_str()), client.getSocketFd()) == -1) return -1;

    // log the request to the server
    writeLog(to_string(((Request*)req)->getRequestId())+ ": Requesting \"" + request.gethost() + "\" from " + ((Request*)req)->getIpAddress() + " @ " + Time::getCurrentTime());

    // recv head message from the server
    string server_msg;
    if((server_msg = recvResponse(client)).empty()) return -1;

    // HTTPResponse parse and log
    HTTPResponse httpResponse(server_msg);
    writeLog(to_string(((Request*)req)->getRequestId())+": HTTPResponse \"" + httpResponse.getLine()  + Time::getCurrentTime());

    // TODO cache
    // check if chunked data
    if(httpResponse.isChunked()){
        handleChunked(req, client, server_msg);
    } else{


    }


}

/**
 * Handle the chunked data
 * @param req request object
 * @param client object
 * @param server_msg received
 */
void ProxyService::handleChunked(const void *req, const Client &client,
                                 string &server_msg) {// while loop receive server's next response and send back to the client until server close
    while (true) {
        Server::trySendMessage(server_msg, ((Request*)req)->getFd());
        if((server_msg = recvResponse(client)).empty()) break;
    }
}

/**
 * Recieve the HTTP Resposne Head
 * @param client object
 * @return
 */
string ProxyService::recvResponse(const Client &client) {
    char server_msg[MAX_TCP_LEN];
    int server_msg_len = 0;
    if((server_msg_len = Client::tryRecvMessage(server_msg, 0, client.getSocketFd())) <= 0){
        return {};
    }
    return string(server_msg, server_msg_len);
}



