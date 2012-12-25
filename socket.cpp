#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#define EXTERN

#include "socket.h"
#include "log.h"

int listenfd = -1, epollfd = -1;

void sock_init() {
    __log;

    socke_accept = [](int, sockaddr_in) {};
    socke_rcv = [](int) {};


    if ((epollfd = epoll_create(10)) < 0)
        logger.raise("Cannot epoll QQ");
}

void sock_final() {
    __log;

    logger.print("Holy shit");
}

void sock_listen(unsigned short port) {
    __log;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw logger.errmsg("canot create socket QQ");

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
        throw logger.errmsg("JIZZ, CANNOT BIND");
    if (listen(listenfd, 10) < 0)
        throw logger.errmsg("listen???");

    sock_watch(listenfd);
}

void sock_watch(int fd) {
    __log;

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) < 0)
        throw logger.errmsg("Cannot add fd to epoll");
}

void sock_unwatch(int fd) {
    __log;

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event) < 0)
        throw logger.errmsg("Cannot remove fd from epoll");
}

void sock_loop() {
    __log;

    epoll_event events[10];
    int nfds;

    for (;;) {
        if ((nfds = epoll_wait(epollfd, events, 10, -1)) < 0)
            throw logger.errmsg("epoll waittttttt");

        for (int i = 0; i < nfds; ++i) {

            if (events[i].data.fd != listenfd) {

                socke_rcv(events[i].data.fd);

            } else {

                struct sockaddr_in client_address;
                socklen_t length = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*) &client_address, &length);
                if (connfd < 0)
                    throw logger.errmsg("Cannot accept connection");

                socke_accept(connfd, client_address);
                socke_rcv(connfd);
            }
        }
    }
}
