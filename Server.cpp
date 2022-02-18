//
// Created by X G on 2/11/22.
//
/**
 * Ref: ECE650 tpc_example and beej's Notebook
 */
#include <arpa/inet.h>
#include <fstream>
#include "Server.h"

Server::Server(int port) {
    this->port = port;
    initHit(); // init the hit
    getAddress(); // get addinfo
    createSocket(); // Create socket
    bindPort(); // bind the port
}

void Server::initHit() {
    memset(&this->hit, 0, sizeof(this->hit));
    this->hit.ai_family = AF_UNSPEC;
    this->hit.ai_socktype = SOCK_STREAM;
    this->hit.ai_flags = AI_PASSIVE;
}

int Server::getAddress() {
    if(getaddrinfo(nullptr, std::to_string(this->port).c_str(), &this->hit, &this->res) != 0) return printError("Error: cannot get address info for host");
    return 0;
}

int Server::createSocket() {
    this->socket_fd = socket(this->res->ai_family, this->res->ai_socktype, this->res->ai_protocol);
    if(this->socket_fd == -1) printError("Error: cannot create socket");
    return this->socket_fd;
}

int Server::printError(std::string error) {
    std::cerr << "(no-id):  " <<  error << std::endl;
    return -1;
}

int Server::bindPort() {
    int yes = 1;
    int status = setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));   //allow address reuse
    status = ::bind(this->socket_fd, this->res->ai_addr, this->res->ai_addrlen);
    if (status == -1) return printError("Error: cannot bind socket");
    // Free the ans ll
    freeaddrinfo(this->res);
    return 0;
}

int Server::listen() {
    // accept incoming connections, define a queue limit for incoming connections
    if (::listen(this->socket_fd, 100) == -1) return printError( "Error: cannot listen on socket");
    return 0;
}

int Server::tryListen() {
    return listen();
}


/**
 * Get socket address
 * @param sa
 * @return
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int Server::accept() {
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr;
    socklen_t socket_addr_len = sizeof(their_addr);
    int client_connection_fd;

    client_connection_fd = ::accept(socket_fd, (struct sockaddr *)&their_addr, &socket_addr_len);

    if (client_connection_fd == -1) {
        return printError("Error: cannot accept connection on socket");
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);

    // Record the player's connection_fd
    this->currClientFd = (client_connection_fd);

    // Record player's ip address
    struct sockaddr_in * addr = (struct sockaddr_in *)&their_addr;
    this->currClientIp = inet_ntoa(addr->sin_addr);
    return client_connection_fd;
}

const int &Server::getBrowserFd() const {
    return this->currClientFd;
}

void Server::close() {
    ::close(this->socket_fd);
}

int Server::tryRecvMessage(char *message, int mode, int fd) {
    int numbytes = 0;
    if ((numbytes = recv(fd, message, MAX_TCP_LEN - 1, mode)) == -1) {
        perror("recv");
        return -1;
    }
    message[numbytes] = '\0';
    return numbytes;
}


__attribute__((unused)) int Server::getPortNum() {
    struct sockaddr_in sin{};
    socklen_t len = sizeof(sin);
    if (getsockname(this->socket_fd, (struct sockaddr *)&sin, &len) == -1) printError("Error: cannot getsockname");
    return ntohs(sin.sin_port);
}

__attribute__((unused)) int Server::getPort() const {
    return port;
}

int Server::trySendMessage(char message[MAX_TCP_LEN], int fd) {
    int len;
    len = strlen(message);
    if (Server::sendall(fd, message, &len) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", len);
        return -1;
    }
    return 0;
}

int Server::tryAccept() {
    return this->currClientFd = accept();
}

const std::string &Server::getCurrBrowserIp() const {
    return currClientIp;
}

/**
 * Static Helper funcion send all message
 * @param s socket
 * @param buf buffer
 * @param len
 * @return
 */
 int Server::sendall(int s, char *buf, int *len)
{
    int total = 0;
    int bytesleft = *len;
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total;

    return n==-1?-1:0;
}
