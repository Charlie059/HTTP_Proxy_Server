//
// Created by X G on 2/12/22.
//

#ifndef PROJECT3_CLIENT_H
#define PROJECT3_CLIENT_H
#define MAX_TCP_LEN 65535
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
/**
 * Ref: ECE650 tpc_example and beej's Notebook
 */
class Client {
public:
    int getSocketFd() const;
private:
    int socket_fd;
    int errorCode;
public:
    int getErrorCode() const;

private:
    struct addrinfo hit;
    struct addrinfo *res; // ll
    const char *hostname;
    int port;
public:
    explicit Client(const char * hostname, int port);
    static int trySendMessage(char *message, int fd);
    static int tryRecvMessage(char *message, int mode, int fd);    // Try to receive the message on the port
    void close() const;
protected:
    void initHit(); // init the hit
    int getAddress(); // get the address
    int createSocket(); // Create a fd_socket
    int connectSocket(); // Connect to the socket
    int printError(std::string error) const; // Print Error
    static int sendall(int s, char *buf, int *len);


};


#endif //PROJECT3_CLIENT_H
