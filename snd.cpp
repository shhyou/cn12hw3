#include <cstdio>
#include <cstdint>
#include <string>
#include <map>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "socket.h"
#include "icmp.h"
#include "log.h"

using std::map;
using std::string;

const int sizeof_buffer = 480; /* too large, icmp can send at most 512 bytes */

raw_t icmp;
map<int, tunnel_t> fd_tunnel;
map<tunnel_t, int> tunnel_fd;

void ircv(tunnel_t tnl, void *buf, size_t len) {
    __log;

    int fd = tunnel_fd[tnl];

    ssize_t sent = send(fd, buf, len, 0);
    if (sent < 0)
        throw logger.errmsg("Cannot send data to fd %d", fd);

    logger.print("Forward data from icmp port %u to %d", tnl.port, fd);
}

auto empty_func = [](tunnel_t, const char*, uint16_t) {};

void iclose(tunnel_t tnl) {
    __log;

    logger.print("Closing fd %d", tunnel_fd[tnl]);
    close(tunnel_fd[tnl]);
    fd_tunnel.erase(tunnel_fd[tnl]);
    tunnel_fd.erase(tnl);
}

int main(int argc, char *argv[]) {
    __log;

    const char *target, *proxy_ip = "0.0.0.0";
    unsigned short port, target_port;

    try {
        if (argc != 5) {
            puts("Usage: ./snd target_name target_port local_listen_port remote_proxy_ip");
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
        sock_listen(port);

        sock_watch(icmp.fd);

        socke_accept = [target, target_port](int fd, sockaddr_in addr) {
            __logl;

            sock_watch(fd);
            fd_tunnel[fd] = icmp_create(icmp, target, target_port);
            tunnel_fd[fd_tunnel[fd]] = fd;

            logger.print("New connection from %u.%u.%u.%u, fd=%d. icmp_create with port %u",
                    addr.sin_addr.s_addr&0xff, (addr.sin_addr.s_addr>>8)&0xff,
                    (addr.sin_addr.s_addr>>16)&0xff, (addr.sin_addr.s_addr>>24)&0xff,
                    fd, fd_tunnel[fd].port);
        };

        socke_closed = [](int fd) {
            __logl;

            logger.print("Socket %d closed", fd);
            icmp_close(icmp, fd_tunnel[fd]);
            sock_unwatch(fd);
            close(fd);
            tunnel_fd.erase(fd_tunnel[fd]);
            fd_tunnel.erase(fd);
        };

        socke_rcv = [](int fd) {
            __logl;

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
                    logger.eprint("Unexpected closure of fd %d", fd);
                    socke_closed(fd);
                }

                logger.print("Forward data from %d to icmp port %u", fd, fd_tunnel[fd].port);
            }
        };

        logger.print("proxy client start pid=%d", getpid());
        sock_loop();
    } catch (const string& e) {
        logger.eprint("Unhandled exception: %s", e.c_str());
    }
    sock_final();
    return 0;
}

