//
// Created by X G on 2/12/22.
//

#ifndef PROJECT3_CLIENT_H
#define PROJECT3_CLIENT_H
#define MAXDATASIZE 65535
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
/**
 * Ref: ECE650 tpc_example and beej's Notebook
 */
class Client {
private:
    int socket_fd;
public:
    int getSocketFd() const;

private:
    struct addrinfo hit;
    struct addrinfo *res; // ll
    const char *hostname;
    const char *port;
public:
    explicit Client(const char * hostname, const char * port);
    int trySendMessage(char *message);
    int tryRecvMessage(char *message, int mode);    // Try to receive the message on the port
    void close();
protected:
    void initHit(); // init the hit
    void getAddress(); // get the address
    int createSocket(); // Create a fd_socket
    void connectSocket(); // Connect to the socket
    void printError(std::string error) const; // Print Error
    static int sendall(int s, char *buf, int *len);


};


#endif //PROJECT3_CLIENT_H
