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
        return 0;
    } else{
        // get the content length
        int content_len = httpResponse.getContentLength();
//        int content_len = getLength(const_cast<char *>(server_msg.c_str()), server_msg.length());  //get content length
        if(content_len != -1){
//            std::string msg = recvAllResponse(client, const_cast<char *>(server_msg.c_str()), server_msg.length(), content_len);
            std::string msg = recvAllResponse(client, server_msg, content_len);
//            std::string msg = recieveFromServer(client.getSocketFd(), const_cast<char *>(server_msg.c_str()), server_msg.length(), content_len);
//            std::vector<char> large_msg;
//            for (size_t i = 0; i < msg.length(); i++) {
//                large_msg.push_back(msg[i]);
//            }
//            const char * send_msg = large_msg.data();
//            send(((Request*)req)->getFd(), send_msg, msg.length(), 0);

            std::vector<char> large_msg(msg.begin(), msg.end());
            const char * send_msg = large_msg.data();
            send(((Request*)req)->getFd(), send_msg, large_msg.size(), 0);
        } else{
            send(((Request*)req)->getFd(), server_msg.c_str(), server_msg.length(), 0);
        }

        // Cache the Response

        return 0;
    }


}


//TODO MAY change in future
/**
 * Base on the content-length fild
 * @param client
 * @param server_msg
 * @param mes_len
 * @param contentLength
 * @return
 */
//string ProxyService::recvAllResponse(Client client, char * server_msg, int headLength, int contentLength) {
//    int curr_len = headLength;
//    int len = 0;
//    std::string recv_msg_str(server_msg, curr_len);
//    while(curr_len < contentLength) {
//        char recv_msg[65536] = {0};
//        if ((len = ( Client::recvMessage(client.getSocketFd(), recv_msg, sizeof(recv_msg))))  <= 0) break;
//        std::string temp(recv_msg, len);
//        recv_msg_str += temp;
//        curr_len += len;
//    }
//    return recv_msg_str;
//}

string ProxyService::recvAllResponse(Client client, string server_meg, int contentLength) {
    char * server_meg_char = const_cast<char *>(server_meg.c_str());
    int curr_len = server_meg.length();
    int len = 0;
    std::string recv_msg_str(server_meg_char, curr_len);
    while(curr_len < contentLength) {
        char recv_msg[65536] = {0};
        if ((len = ( Client::recvMessage(client.getSocketFd(), recv_msg, sizeof(recv_msg))))  <= 0) break;
        std::string temp(recv_msg, len);
        recv_msg_str += temp;
        curr_len += len;
    }
    return recv_msg_str;
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
 * Recieve the HTTP Piece of Response
 * @param client object
 * @return
 */
string ProxyService::recvResponse(const Client &client) {
    char server_msg[MAX_TCP_LEN] = {0};
    int server_msg_len = 0;
    if((server_msg_len = Client::tryRecvMessage(server_msg, 0, client.getSocketFd())) <= 0){
        return {};
    }
    return string(server_msg, server_msg_len);
}

//TODO del
std::string ProxyService::recieveFromServer(int send_fd,
                                            char * server_msg,
                                            int mes_len,
                                            int content_len) {
    int total_len = 0;
    int len = 0;
    std::string msg(server_msg, mes_len);

    while (total_len < content_len) {
        char new_server_msg[65536] = {0};
        if ((len = recv(send_fd, new_server_msg, sizeof(new_server_msg), 0)) <= 0) {
            break;
        }
        std::string temp(new_server_msg, len);
        msg += temp;
        total_len += len;
    }
    return msg;
}

//TODO del
int ProxyService::getLength(char * server_msg, int mes_len) {

    std::string msg(server_msg, mes_len);
    std::transform(msg.begin(),msg.end(),msg.begin(),::tolower);
    size_t pos;
    if ((pos = msg.find("content-length: ")) != std::string::npos) {
        size_t head_end = msg.find("\r\n\r\n");

        int part_body_len = mes_len - static_cast<int>(head_end) - 8;
        size_t end = msg.find("\r\n", pos);
        std::string content_len = msg.substr(pos + 16, end - pos - 16);
        int num = 0;
        for (size_t i = 0; i < content_len.length(); i++) {
            num = num * 10 + (content_len[i] - '0');
        }
        return num - part_body_len - 4;
    }
    return -1;
}