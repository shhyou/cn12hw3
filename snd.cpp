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

int main(int argc, char *argv[]) {
    const char *target;
    short port;

    if (argc < 3) {
        puts("Usage: ./snd target listen_port");
        puts("Defaulted to ptt.cc, listening at 5000");
        target = "ptt.cc";
        port = 5000;
    } else {
        target = argv[1];
        port = (short) atoi(argv[2]);
    }

    icmp = icmp_socket();

    sock_init();
    sock_listen(port); /* for example */

    sock_watch(icmp);

    socke_accept = [&fd_port](int fd, sockaddr_in addr) {
        sock_watch(fd);
        fd_port[fd] = icmp_create(target, 23);
    };

    socke_rcv = [&fd_port](int fd) {
        if (fd == icmp) {
            icmp_rcv();
        } else {

        }
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

