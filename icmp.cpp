#include <stdint.h>
#include <sys/socket.h>

#include "icmp.h"

#define ICMP_CREATE 1
#define ICMP_CREATED 2
#define ICMP_CLOSE 3

/* port == 0 for control messages */

struct __attribute__((packed)) pkt_t {
    uint32_t port;
    uint8_t data[];
};

int icmp_socket() {}

uint32_t icmp_create(const char* name, unsigned short foreign_port) {}

void icmp_close(const uint32_t port) {}

size_t icmp_snd(uint32_t port, const void* buf, const size_t len) {}

void icmp_rcv(
        function<void(void*, size_t)> ircv,
        function<void(uint32_t, const char*)> iaccept,
        function<void(uint32_t)> iclose) {
}


