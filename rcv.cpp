#include <cstdio>
#include <cstdint>
#include <map>
#include <string>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "socket.h"
#include "icmp.h"
#include "log.h"

using std::map;
using std::string;

raw_t icmp;
map<int, tunnel_t> fd_tunnel;
map<tunnel_t, int> tunnel_fd;

int main() {
    __log;

    try {
        icmp = icmp_socket("0.0.0.0", 0); /* ping reply */

        sock_init();
        sock_watch(icmp.fd);

        socke_closed = [](int fd) {
            icmp_close(icmp, fd_tunnel[fd]);
            sock_unwatch(fd);
            close(fd);
            tunnel_fd.erase(fd_tunnel[fd]);
            fd_tunnel.erase(fd);
        };

        socke_rcv = [](int fd) {
            __logl;

            char buffer[2048];
            if (fd != icmp.fd) {
                ssize_t rcv = recv(fd, buffer, 480, 0);
                if (rcv < 0)
                    throw logger.errmsg("received jizz from port %d", fd_tunnel[fd].port);
                if (rcv == 0) {
                    logger.eprint("Unexpected closure of fd %d", fd);
                    socke_closed(fd);
                    return;
                }
                tunnel_t tnl = fd_tunnel[fd];
                icmp_snd(icmp, tnl, buffer, rcv);
                logger.print("Forward data from %d to icmp {port=%u, dst=%u.%u.%u.%u, echoid=%u}",
                        fd,
                        tnl.port,
                        tnl.dst&0xff, (tnl.dst>>8)&0xff,
                        (tnl.dst>>16)&0xff, (tnl.dst>>24)&0xff,
                        tnl.echoid);
                return;
            }

            icmp_rcv(icmp,
                    [](tunnel_t tnl, void* buf, size_t len) { /* ircv */
                       __logl;

                        int tfd = tunnel_fd[tnl];

                        ssize_t sent = send(tfd, buf, len, 0);
                        if (sent < 0)
                            throw logger.errmsg("Failed to send data to fd %d", tfd);

                        logger.print("Forward data from icmp port %u to %d", tnl.port, tfd);
                    },

                    [](tunnel_t tnl, const char* name, uint16_t port) { /* iaccept */
                        __logl;

                        sockaddr_in sin;
                        hostent *h;
                        memset(&sin, 0, sizeof(sin));

                        h = gethostbyname(name);
                        if (h == NULL)
                            throw logger.errmsg("Cannot get IP of %s", name);

                        sin.sin_family = AF_INET;
                        sin.sin_port = htons(port);
                        sin.sin_addr.s_addr = *(in_addr_t*)h->h_addr_list[0];

                        int s = socket(AF_INET, SOCK_STREAM, 0);
                        if (s < 0)
                            throw logger.errmsg("Cannot create new socket");

                        if (connect(s, (sockaddr*)&sin, sizeof(sin)) < 0) {
                            close(s);
                            throw logger.errmsg("Cannot connect to %s(%d.%d.%d.%d):%u, fd=%d",
                                name,
                                sin.sin_addr.s_addr&0xff,
                                (sin.sin_addr.s_addr>>8)&0xff,
                                (sin.sin_addr.s_addr>>16)&0xff,
                                (sin.sin_addr.s_addr>>24)&0xff,
                                (uint32_t)port,
                                s);
                        }

                        sock_watch(s);

                        fd_tunnel[s] = tnl;
                        tunnel_fd[tnl] = s;

                        logger.print("New connection to %s(%d.%d.%d.%d):%u, fd=%d",
                                name,
                                sin.sin_addr.s_addr&0xff,
                                (sin.sin_addr.s_addr>>8)&0xff,
                                (sin.sin_addr.s_addr>>16)&0xff,
                                (sin.sin_addr.s_addr>>24)&0xff,
                                (uint32_t)port,
                                s);
                    },

                    [](tunnel_t tnl) { /* iclose */
                        __logl;

                        int fd = tunnel_fd[tnl];
                        logger.print("Closing fd %d", fd);
                        sock_unwatch(fd);
                        close(fd);
                        tunnel_fd.erase(tnl);
                        fd_tunnel.erase(fd);
                    });
        };

        socke_accept = [](int, sockaddr_in) {
            __logl;
            logger.eprint("WTF");
        };

        logger.print("proxy server start pid=%d", getpid());
        sock_loop();
    } catch (const string& e) {
        logger.eprint("Unhandled exception: %s", e.c_str());
    }
    sock_final();
    return 0;
}

