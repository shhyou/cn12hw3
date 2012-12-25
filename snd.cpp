// TODO: proxy server address (currently 0.0.0.0)
#include <cstdio>
#include <cstdint>
#include <map>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "socket.h"
#include "icmp.h"
#include "log.h"

using std::map;

const int sizeof_buffer = 2048;

raw_t icmp;
map<int, uint32_t> fd_port;
map<uint32_t, int> port_fd;

void ircv(sockaddr_in, uint16_t, uint32_t port, void *buf, size_t len) {
    __log;

    int fd = port_fd[port];

    ssize_t sent = send(fd, buf, len, 0);
    if (sent < 0)
        throw logger.errmsg("Failed to send data to fd %d", fd);

}

auto empty_func = [](sockaddr_in, uint16_t, uint32_t, const char*, uint16_t) {};

void iclose(sockaddr_in, uint16_t, uint32_t port) {
    __log;

    close(port_fd[port]);
    fd_port.erase(port_fd[port]);
    port_fd.erase(port);
}

int main(int argc, char *argv[]) {
    __log;

    const char *target;
    short port;

    if (argc < 3) {
        puts("Usage: ./snd target listen_port");
        logger.print("Defaulted to ptt.cc, listening at 5000");
        target = "ptt.cc";
        port = 5000;
    } else {
        target = argv[1];
        port = (short) atoi(argv[2]);
    }

    icmp = icmp_socket("0.0.0.0", 8); // send ping request

    sock_init();
    sock_listen(port); /* for example */

    sock_watch(icmp.fd);

    socke_accept = [target](int fd, sockaddr_in addr) {
        __log;

        sock_watch(fd);
        fd_port[fd] = icmp_create(icmp, target, 23);
        port_fd[fd_port[fd]] = fd;

        logger.print("New connection from %d", ntohs(addr.sin_port));
    };


    socke_rcv = [](int fd) {
        __log;

        char buffer[sizeof_buffer];
        if (fd == icmp.fd) {

            icmp_rcv(icmp, ircv, empty_func, iclose);

        } else {

            ssize_t recved = recv(fd, &buffer, sizeof_buffer, 0);
            if (recved < 0) {
                throw logger.errmsg("Error receving data from port %d", fd_port[fd]);
            } else if (recved != 0) {
                icmp_snd(icmp, fd_port[fd], buffer, recved);
            } else {
                icmp_close(icmp, fd_port[fd]);
                close(fd);
            }

        }
    };

    sock_loop();
    sock_final();
    return 0;
}

