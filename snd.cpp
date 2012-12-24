#include <cstdio>
#include <cstdint>
#include <map>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "socket.h"
#include "icmp.h"

using std::map;

int icmp;
map<int, uint32_t> fd_port; 

int main() {
    icmp = icmp_socket();

    sock_init();
    sock_listen(5000); /* for example */

    sock_watch(icmp);

    socke_accept = [&fd_port](int fd, sockaddr_in addr) {
        sock_watch(fd);
        fd_port[fd] = icmp_create("ptt.cc", 23);
    };

    socke_rcv = [&fd_port](int fd) {
        // if fd is not icmp...
        //   recv...
        //   icmp_snd...
        // else
        //   icmp_rcv...
    };

    sock_loop();
    sock_final();
    return 0;
}

