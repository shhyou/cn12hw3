#include <cstdio>
#include <unistd.h>
#include <string>

using namespace std;

#include "icmp.h"

raw_t icmp;

void ircv(tunnel_t tnl, void* buf, size_t len) {
    printf("ircv    tnl.dst = %u, tnl.port = %u, tnl.echoid = %u\n", tnl.dst, tnl.port, tnl.echoid);
    printf("        len = %u\n", len);
    icmp_snd(icmp, tnl, "send back", 10);
}

void iaccpet(tunnel_t tnl, const char *name, uint16_t port) {
    printf("iaccept tnl.dst = %u, tnl.port = %u, tnl.echoid = %u\n", tnl.dst, tnl.port, tnl.echoid);
    printf("        name = '%s', port = %u\n", name, port);
}

void iclose(tunnel_t tnl) {
    printf("iclose  tnl.dst = %u, tnl.port = %u, tnl.echoid = %u\n", tnl.dst, tnl.port, tnl.echoid);
}

int main() {
    try {
        icmp = icmp_socket("127.0.0.1", 8); // ping request
        tunnel_t tnl = icmp_create(icmp, "ptt.cc", 23);
        
        printf("getpid = %d\n", getpid());
        printf("icmp.fd = %d\n", icmp.fd);
        printf("tnl.dst = %u, tnl.port = %u, tnl.echoid = %u\n", tnl.dst, tnl.port, tnl.echoid);

        for (int i = 0; i < 4; i++) {
            icmp_rcv(icmp, ircv, iaccpet, iclose);
        }
        icmp_close(icmp, tnl);

        close(icmp.fd);
    } catch (const string& e) {
        printf("%s\n", e.c_str());
    }
    return 0;
}


