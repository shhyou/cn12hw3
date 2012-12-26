#include <cstdio>
#include <cstdint>
#include <map>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "socket.h"
#include "icmp.h"
#include "log.h"

using std::map;

raw_t icmp;
map<int, tunnel_t> fd_tunnel;
map<tunnel_t, int> tunnel_fd;

int main() {
    __log;

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
            icmp_snd(icmp, fd_tunnel[fd], buffer, rcv);
			return;
        }

        icmp_rcv(icmp,
                [](tunnel_t tnl, void* buf, size_t len) { /* ircv */
                    __logl;

                    int tfd = tunnel_fd[tnl];

                    ssize_t sent = send(tfd, buf, len, 0);
                    if (sent < 0)
                        throw logger.errmsg("Failed to send data to fd %d", tfd);
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
                    sin.sin_port = port;
                    sin.sin_addr.s_addr = *(in_addr_t*)h->h_addr_list[0];

                    int s = socket(AF_INET, SOCK_STREAM, 0);
                    if (s < 0)
                        throw logger.errmsg("Cannot create new socket");

                    if (connect(s, (sockaddr*)&sin, sizeof(sin)) < 0) {
                        close(s);
                        throw logger.errmsg("Cannot connectto %s:%u", name, (uint32_t)port);
                    }

                    sock_watch(s);

                    fd_tunnel[s] = tnl;
                    tunnel_fd[tnl] = s;

                    logger.print("New connection to %s:%u", name, (uint32_t)port);
                },

                [](tunnel_t tnl) { /* iclose */
                    socke_closed(tunnel_fd[tnl]);
                });
    };

    socke_accept = [](int, sockaddr_in) {
        __logl;
        logger.eprint("WTF");
    };

    logger.print("proxy server start");
    sock_loop();
    sock_final();
    return 0;
}

