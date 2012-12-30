#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <string>

using namespace std;

#include "icmp.h"

raw_t icmp;
tunnel_t atnl;

void ircv(tunnel_t tnl, void* buf, size_t len) {
    printf("ircv    tnl.dst = %u, tnl.port = %u, tnl.echoid = %u\n", tnl.dst, tnl.port, tnl.echoid);
    printf("        len = %u\n", len);
    if (memcmp(&tnl, &atnl, sizeof(tunnel_t)) == 0) {
        printf("        wow, get meesage from atnl! sending 'how are you' back\n");
        icmp_snd(icmp, tnl, "how are you", 12);
    } else {
        printf("        ??? unknown tunnel\n");
    }
}

void iaccpet(tunnel_t tnl, const char *name, uint16_t port) {
    printf("iaccept tnl.dst = %u, tnl.port = %u, tnl.echoid = %u\n", tnl.dst, tnl.port, tnl.echoid);
    printf("        name = '%s', port = %u\n", name, port);
    printf("        sending 'Hello World' back...\n");
    atnl = tnl;
    icmp_snd(icmp, atnl, "Hello World", 12);
}

void iclose(tunnel_t tnl) {
    printf("iclose  tnl.dst = %u, tnl.port = %u, tnl.echoid = %u\n", tnl.dst, tnl.port, tnl.echoid);
}

int main() {
    try {
        icmp = icmp_socket("0.0.0.0", 0); // ping request
        
        printf("getpid = %d\n", getpid());
        printf("icmp.fd = %d\n", icmp.fd);

        while (1) {
            icmp_rcv(icmp, ircv, iaccpet, iclose);
        }

        close(icmp.fd);
    } catch (const string& e) {
        printf("%s\n", e.c_str());
    }
    return 0;
}


