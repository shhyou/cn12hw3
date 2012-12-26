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
map<int, tunnel_t> fd_tunnel;
map<tunnel_t, int> tunnel_fd;

void ircv(tunnel_t tnl, void *buf, size_t len) {
    __log;

    int fd = tunnel_fd[tnl];

    ssize_t sent = send(fd, buf, len, 0);
    if (sent < 0)
        throw logger.errmsg("Failed to send data to fd %d", fd);

}

auto empty_func = [](tunnel_t, const char*, uint16_t) {};

void iclose(tunnel_t tnl) {
    __log;

    close(tunnel_fd[tnl]);
    fd_tunnel.erase(tunnel_fd[tnl]);
    tunnel_fd.erase(tnl);
}

int main(int argc, char *argv[]) {
    __log;

    const char *target, *proxy_ip = "0.0.0.0";
    unsigned short port, target_port;

    if (argc != 5) {
        puts("Usage: ./snd target_ip target_port local_listen_port remote_proxy_ip");
        puts("JIZZ");
        exit(0);
    } else {
        target = argv[1];
        target_port = (unsigned short) atoi(argv[2]);
        port = (unsigned short) atoi(argv[3]);
        proxy_ip = argv[4];
    }

    icmp = icmp_socket(proxy_ip, 8); // send ping request

    sock_init();
    sock_listen(port); /* for example */

    sock_watch(icmp.fd);

    socke_accept = [target, target_port](int fd, sockaddr_in addr) {
        __log;

        sock_watch(fd);
        fd_tunnel[fd] = icmp_create(icmp, target, target_port);
        tunnel_fd[fd_tunnel[fd]] = fd;

        logger.print("New connection from %u.%u.%u.%u",
                addr.sin_addr.s_addr&0xff, (addr.sin_addr.s_addr>>8)&0xff,
                (addr.sin_addr.s_addr>>16)&0xff, (addr.sin_addr.s_addr>>24)&0xff);
    };


    socke_rcv = [](int fd) {
        __log;

        char buffer[sizeof_buffer];
        if (fd == icmp.fd) {

            icmp_rcv(icmp, ircv, empty_func, iclose);

        } else {

            ssize_t recved = recv(fd, &buffer, sizeof_buffer, 0);
            if (recved < 0) {
                throw logger.errmsg("Error receving data from port %u", fd_tunnel[fd].port);
            } else if (recved != 0) {
                icmp_snd(icmp, fd_tunnel[fd], buffer, recved);
            } else {
                icmp_close(icmp, fd_tunnel[fd]);
                close(fd);
            }

        }
    };

    sock_loop();
    sock_final();
    return 0;
}

