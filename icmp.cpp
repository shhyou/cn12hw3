#include <stdint.h>
#include <sys/socket.h>

#include "icmp.h"

#define ICMP_CREATE 1
#define ICMP_CREATED 2
#define ICMP_CLOSE 3

#define ICMP_MAGIC 0x514B1E55  // tmt514's BlESS

#define ICMP_LEN 512

/* port == 0 for control messages */

struct __attribute__((packed)) pkt_t {
	uint32_t magic;
    uint32_t port;
	union __attribute__((packed)) icmpload_t {
		struct __attribute__((packed)) icmpmsg_t {
			uint32_t len;
			uint8_t data[ICMP_LEN];
		} msg;
		struct __attribute__((packed)) icmpctl_t {
			uint32_t type;
			uint32_t port;
			uint16_t dstport;
			uint8_t dstname[ICMP_LEN - 4 - 2];
		} ctl;
	} load;
	uint32_t magic2;
};

int icmp_socket() { return -1; }

uint32_t icmp_create(const char* name, unsigned short foreign_port) {}

void icmp_close(const uint32_t port) {}

size_t icmp_snd(int fd, uint32_t port, const void* buf, const size_t len) {}

void icmp_rcv(int fd,
        function<void(uint32_t, void*, size_t)> ircv,
        function<void(uint32_t, const char*)> iaccept,
        function<void(uint32_t)> iclose) {
}


