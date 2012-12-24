#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>

#define EXTERN

#include "socket.h"

void sock_init() {
    socke_accept = [](int, sockaddr_in) {};
    socke_closed = [](int) {};
    socke_rcv = [](int) {};
}

/* something array of listening sockets created by sock_listen */
int lsn_arr[8];
void sock_final() {}
void sock_listen(unsigned short port) {}
void sock_watch(int fd) {}
void sock_loop() {}
