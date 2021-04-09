//
//  Server.cpp
//  LAB2
//
//  Created by 钱依柔 on 6/8/19.
//  Copyright © 2019 钱依柔. All rights reserved.
//

#include "Server.hpp"

#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <sstream>


#define PORT "3729"  // 3300 + 429 (USC ID: 4354 7164 29)
#define BACKLOG 10     // how many pending connections queue will hold
#define MAXDATASIZE 1024 // max number of bytes

using namespace std;

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


string int_to_string (int num) {
    char tmp[15] = {0};
    snprintf(tmp, 15, "%d", num);
    return tmp;
}

string random_transaction_ID () {
    int id = rand() % 255;
    return int_to_string (id);
}

string random_IP_address (string randID) {
    string address = "";
    int tmp = 0;
    
    for (int i = 0; i < 3; i++) {
        tmp = rand() % 255;
        address = int_to_string (tmp) + "." + address;
    }
    return address + randID;
}


int main(void) {
    
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    char buffer[MAXDATASIZE];
    int rv;
    int numbytes;
    
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    printf("server: waiting for connections...\n");
    
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        struct sockaddr *sa = (struct sockaddr *)&their_addr;
        if (sa->sa_family == AF_INET)
        {
            inet_ntop(their_addr.ss_family, &(((struct sockaddr_in*)sa)->sin_addr), s, sizeof s);
        }
        else
        {
            inet_ntop(their_addr.ss_family, &(((struct sockaddr_in6*)sa)->sin6_addr), s, sizeof s);
        }
        
        //inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
        
        if (!fork()) { // this is the child process
            close(sockfd);// child doesn't need the listener
            
            // receive requests from the client
            if ((numbytes = recv(new_fd, buffer, MAXDATASIZE-1, 0)) > 0) {
                printf("server: receive transaction ID from client: %s\n", buffer);
            }
            
            // offer phase
            string strAddr = random_IP_address(buffer);
            string strID = random_transaction_ID();
            string strMsg = strAddr + "#" + strID;
            strcpy(buffer, strMsg.c_str());
            if ((numbytes = send(new_fd, buffer , MAXDATASIZE-1, 0)) > 0) {
                printf("server: offer IP address to client: %s\n", strAddr.c_str());
                printf("server: generate new transaction ID: %s\n", strID.c_str());
            }
            
            // acknlowledge phase
            if ((numbytes = recv(new_fd, buffer, MAXDATASIZE-1, 0)) > 0) {
                printf("server: acknowledge with new transaction ID: %s\n", buffer);
            }
            
            strID = random_transaction_ID();
            strcpy(buffer, strID.c_str());
            if ((numbytes = send(new_fd, buffer, MAXDATASIZE-1, 0)) == -1) {
                perror("send");
            }
            printf("server: send ACK and IP address to client: %s\n", strAddr.c_str());
            printf("server: generate new transaction ID: %s\n", buffer);
            
            close(new_fd);
            exit(0);
        }        
        close(new_fd);  // parent doesn't need this
    }
    return 0;
}
