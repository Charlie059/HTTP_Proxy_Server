//
// Created by X G on 2/11/22.
//

#ifndef PROJECT3_SERVER_H
#define PROJECT3_SERVER_H
#define MAX_TCP_LEN 65535 // max TCP size

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
/**
 * Ref: ECE650 tpc_example and beej's Notebook
 */
class Server {
public:
    int getErrorCode() const;
private:
    int errorCode;
    int socket_fd;
    struct addrinfo hit;
    struct addrinfo *res; // ll
    __attribute__((unused)) const char *hostname;
    int port;
    int clientFd;
public:
    int getClientFd() const;

    const std::string &getClientIp() const;

private:
    std::string clientIp;

public:
    /**
     * Server object declare with port number
     * @param port num want to listen
     */
    explicit Server(int port);

    /**
     * Server try listen the port
     * @return -1 indicate error and 0 for success
     */
    int tryListen(); // return the fd_socket

    /**
     * Server try accept one connection from the queue
     * @return -1 indicate error or the fd of new connection
     */
    int tryAccept(); // Server accept one connection from the queue

    /**
     * Close the TCP connection from server side (DO NOT USE TO CLOSE CLIENT FD)
     */
    __attribute__((unused)) void close(); // Close TCP connection

    // Static methods
    /**
     * Try to send piece of message
     * @param message to be sent
     * @param fd indicate the client_fd
     * @return -1 for error, 0 for ok
     */
    static int trySendMessage(std::string message, int fd);

    /**
     * Try receive message and NOT guaranteed all the message is recv. ie. need to call multi times if need
     * @param message buffer
     * @param mode 0
     * @param fd receive from which client fd
     * @return -1 for error, > 0 for ok (0 indicate colse connection)
     */
    static int tryRecvMessage(char *message, int mode, int fd);    // Try to receive the message on the port

    // Getter func

    __attribute__((unused)) int getPort() const; // get itself port number

protected:
    void initHit(); // init the hit
    int getAddress(); // get the address
    int createSocket(); // Create a fd_socket
    int bindPort(); // bind the port with socket
    int listen(); // listen the port
    int accept(); // accept one in queue and return the new fd
    static int printError(std::string error) ; // Print Error
    __attribute__((unused)) int getPortNum();
    __attribute__((unused)) static int sendall(int s, const char *buf, int *len); // Send all message based on the TCP
};


#endif //PROJECT3_SERVER_H
