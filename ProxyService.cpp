//
// Created by X G on 2/14/22.
//

#include "ProxyService.h"
#include "Client.h"
#include "Time.h"
#include "Cache.h"

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile("proxy.log"); // TODO change back
Cache cache(100);

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

    // verifyRequest
    if(verifyRequest(req, httpRequest) == -1) return nullptr;

    // Connect to the Server
    Client client(httpRequest.gethost().c_str(), stoi(httpRequest.getport()));//TODO stoi catch
    if(verifyHostServer(req, client) == -1) return nullptr;

    // If current client request HEAD is CONNECT
    if(httpRequest.getmethod() == "CONNECT"){
        handleConnect(req, httpRequest, client);
    }
    else{ // else if POST or GET
        // TODO TEST

        // if find in cache
        if(!cache.get(httpRequest.getline()).first.empty()){
            vector<char> response = cache.get(httpRequest.getline()).first; // get the full response
            time_t recv_time = cache.get(httpRequest.getline()).second; // get the recv_time
            std::string response_str(response.begin(), response.end());
            HTTPResponse parse(response_str);

            // if the response is no-cache
            if(parse.isNoCache()){
                bool result = revalidate(httpRequest, parse, client);
                if(result == true){ //use cache
                    // log use cache
                    writeLog("Use Cache with " + httpRequest.getline());
                    // return response to client
                    const char * send_msg = response.data();
                    send(((Request*)req)->getFd(), send_msg, response.size(), 0);
                    close(((Request*)req)->getFd()); // close client connection
                    return nullptr;
                }else{
                    handleGet(req, httpRequest, client);
                }
            }else{
                if(isFresh(recv_time, parse)){//checkFresh()
                    // log use cache
                    writeLog("Use Cache with " + httpRequest.getline());
                    const char * send_msg = response.data();
                    send(((Request*)req)->getFd(), send_msg, response.size(), 0);
                    close(((Request*)req)->getFd()); // close client connection
                    return nullptr;
                } else{
                    handleGet(req, httpRequest, client);
                }
            }
            close(((Request*)req)->getFd()); // close client connection
            client.close(); // close the server connnection
            return nullptr;

        }else{ // if not found in cache
            handleGet(req, httpRequest, client);
            close(((Request*)req)->getFd()); // close client connection
            client.close();
            return nullptr;
        }

        // TODO TEST END

    }

    // Close Connection
    close(((Request*)req)->getFd()); // close client connection
    client.close();
    return nullptr;
}

/**
 * find if the cache is fresh
 * @param recv_time
 * @param parse
 * @return true is fresh
 */
bool ProxyService::isFresh(time_t recv_time, const HTTPResponse &parse) {
    int currAge = Time::getCurrentTm() - recv_time;
    if(parse.getSMaxAge() != -1){
        return parse.getSMaxAge() > currAge;
    }else if(parse.getMaxAge() != -1){
        return parse.getMaxAge() > currAge;
    }else if(parse.getExpireTime() != 0){
        return difftime(parse.getExpireTime(), Time::getCurrentTm()) > 0;
    } else{
        // Set estimate lift-time
        return 150 > currAge;
    }
}

/**
 * Revalidate the httpRequest
 * @param httpRequest
 * @param parse
 * @return
 */
bool ProxyService::revalidate(HTTPRequest &httpRequest, const HTTPResponse &parse, Client & client) {
    string msg_to_send;
    if(!parse.getEtag().empty() && !parse.getLastModified().empty()){ //if etag and last-modified both exist
        msg_to_send = HTTPRequest::buildHTTPRequest(httpRequest.getline(), true, true, parse.getEtag(), parse.getLastModified());
    } else if(!parse.getEtag().empty() && parse.getLastModified().empty()){ // if has ETag and no LastModified
        msg_to_send = HTTPRequest::buildHTTPRequest(httpRequest.getline(), true, false, parse.getEtag(), "");
    } else if(parse.getEtag().empty() && !parse.getLastModified().empty()) { // if does not have ETag and have LastModified
        msg_to_send = HTTPRequest::buildHTTPRequest(httpRequest.getline(), false, true, "", parse.getLastModified());
    } else{
        return false;
    }
    // send message to server
    Client::trySendMessage(const_cast<char *>(msg_to_send.c_str()), client.getSocketFd());
    // recv message from server
    string res = recvResponse(client);
    if(HTTPResponse::findNotModified(res)) return true; // use cache
    else return false;
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


/** TODO change in future
 * Handle Connect
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

    // TODO TEST
    if(Client::trySendMessage(const_cast<char *>(request.getRaw().c_str()), client.getSocketFd()) == -1) return -1;

    // log the request to the server
    writeLog(to_string(((Request*)req)->getRequestId())+ ": Requesting \"" + request.gethost() + "\" from " + ((Request*)req)->getIpAddress() + " @ " + Time::getCurrentTime());

    // recv head message from the server
    string server_msg;
    if((server_msg = recvResponse(client)).empty()) return -1;

    // HTTPResponse parse and log
    HTTPResponse httpResponse(server_msg); // TODO when call this HTTPResponse, assign a current time
    writeLog(to_string(((Request*)req)->getRequestId())+": HTTPResponse \"" + httpResponse.getLine()  + Time::getCurrentTime());

    // TODO cache
    // check if chunked data
    if(httpResponse.isChunked()){
        handleChunked(req, client, server_msg);
        return 0;
    } else{
        // get the content length
        int content_len = httpResponse.getContentLength();
        std::string msg = recvAllResponse(client, server_msg, content_len);
        std::vector<char> response_msg(msg.begin(), msg.end());
        const char * send_msg = response_msg.data();
        send(((Request*)req)->getFd(), send_msg, response_msg.size(), 0);


        // TODO HTTPResponse::isCacheable()
        if(httpResponse.isCacheable() && request.getmethod() != "POST"){
            // Cache the Response
            cache.put(request.getline(), std::make_pair(response_msg, httpResponse.getRecvTime()));
            // log info
            string logInfo = to_string(((Request*)req)->getRequestId())+": Cached \"" + httpResponse.getLine()  + Time::getCurrentTime();
            writeLog(logInfo);
        }
            //TODO httpResponse.set_response_raw_data
            //TODO cache value -> httpResponse

        return 0;
    }


}


//TODO MAY change in future
/**
 * Base on the content-length field
 * @param client
 * @param server_msg
 * @param mes_len
 * @param contentLength
 * @return
 */

string ProxyService::recvAllResponse(Client client, string server_meg, int contentLength) {
    char * server_meg_char = const_cast<char *>(server_meg.c_str());
    int curr_len = server_meg.length();
    int len = 0;
    std::string recv_msg_str(server_meg_char, curr_len);
    while(curr_len < contentLength) {
        char recv_msg[65536] = {0};
        if ((len = (Client::recvMessage(client.getSocketFd(), recv_msg, sizeof(recv_msg))))  <= 0) break;
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
