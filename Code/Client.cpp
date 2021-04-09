//
//  Client.cpp
//  LAB2
//
//  Created by 钱依柔 on 6/8/19.
//  Copyright © 2019 钱依柔. All rights reserved.
//

#include "Client.hpp"

#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>

#define PORT "3729"  // 3300 + 429 (USC ID: 4354 7164 29)
#define MAXDATASIZE 1024 // max number of bytes

using namespace std;


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


int main(int argc, char *argv[]) {
    
    srand(time(NULL));
    
    int sockfd, numbytes;
    char buffer[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    struct sockaddr *sa = (struct sockaddr *)p->ai_addr;
    if (sa->sa_family == AF_INET)
    {
        inet_ntop(p->ai_family, &(((struct sockaddr_in*)sa)->sin_addr), s, sizeof s);
    }
    else
    {
        inet_ntop(p->ai_family, &(((struct sockaddr_in6*)sa)->sin6_addr), s, sizeof s);
    }
    
    //inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
    
    freeaddrinfo(servinfo); // all done with this structure
    
    // discover phase
    string strID = random_transaction_ID();
    strcpy(buffer, strID.c_str());
    if ((numbytes = send(sockfd, buffer, MAXDATASIZE-1, 0)) > 0) {
        printf("client: generate transaction ID: %s\n", buffer);
    }
    
    //receive IP address from server
    string strRecvAddr;
    string strRecvID;
    if ((numbytes = recv(sockfd, buffer, MAXDATASIZE-1, 0)) > 0) {
        string msg = string(buffer);
        int divideAddr = msg.find('#');
        strRecvAddr = msg.substr(0, divideAddr);
        strRecvID = msg.substr(divideAddr+1);
        printf("client: get IP address from server: %s\n", strRecvAddr.c_str());
        printf("client: generate new transaction ID: %s\n", strRecvID.c_str());
    }
    
    // request phase
    strID = random_transaction_ID();
    strcpy(buffer, strID.c_str());
    if ((numbytes = send(sockfd, buffer, MAXDATASIZE-1, 0)) > 0) {
        printf("client: want to take this IP address from server: %s\n", strRecvAddr.c_str());
        printf("client: generate new transaction ID: %s\n", buffer);
    }
    
    if ((numbytes = recv(sockfd, buffer, MAXDATASIZE-1, 0)) > 0) {
        printf("client: successfully get IP address: %s\n", strRecvAddr.c_str());
    }
    
    if ((numbytes = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    
    buffer[numbytes] = '\0';
    close(sockfd);
    
    return 0;
}
