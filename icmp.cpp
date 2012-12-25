#include <cstring>
#include <cstdint>
#include <cassert>

#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h> // including everything in the universe
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <linux/ip.h>
#include <linux/icmp.h>

#include "icmp.h"
#include "log.h"

#define ICMP_CREATE 1
#define ICMP_CLOSE 2

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
            uint32_t ctlport;
            uint16_t dstport;
            uint8_t dstname[ICMP_LEN - 4 - 2];
        } ctl;
    } load;
    uint32_t magic2;
};

uint32_t freeport = 1;
uint16_t icmpseq = 0;

void mkpkt_magic(pkt_t& pkt) {
    pkt.magic = ICMP_MAGIC;
    pkt.magic2 = ICMP_MAGIC+1;
}

void unpkt_magic(pkt_t& pkt) {
    __log;

    if (pkt.magic != ICMP_MAGIC ||
            pkt.magic2 != ICMP_MAGIC+1)
        logger.raise("broken icmp message");
}

void mkpkt(pkt_t& pkt, uint32_t port, uint32_t len, const uint8_t data[ICMP_LEN]) {
    memset(&pkt, 0, sizeof(pkt));
    mkpkt_magic(pkt);
    pkt.port = port;
    pkt.load.msg.len = len;
    memcpy(pkt.load.msg.data, data, len);
}

void mkpkt(pkt_t& pkt, uint32_t type, uint32_t ctlport) {
    memset(&pkt, 0, sizeof(pkt));
    mkpkt_magic(pkt);
    pkt.port = 0;
    pkt.load.ctl.type = type;
    pkt.load.ctl.ctlport = ctlport;
}

uint16_t check(uint16_t arr[], size_t len) {
    uint32_t chk = 0;
    for (size_t i = 0; i+2 <= len; i++) {
        chk += arr[i];
        if (chk > 65535) {
            chk++;
            chk &= 0xffff;
        }
    }
    return (chk == 0xffff) ? chk : (chk ^ 0xffff);
}

void icmp_sendto(raw_t icmp, pkt_t& pkt) {
    __log;

    struct icmp_t {
        icmphdr hdr;
        pkt_t pkt;
    } data;

    data.hdr.type = icmp.sndtype;
    data.hdr.code = 0;
    data.hdr.checksum = 0;
    data.hdr.un.echo.id = getpid();
    data.hdr.un.echo.sequence = icmpseq++;

    data.hdr.checksum = check((uint16_t*)&data.hdr, sizeof(data.hdr));

    data.pkt = pkt;

    if (sendto(icmp.fd, &data, sizeof(data), 0, (sockaddr*)&icmp.dst, sizeof(icmp.dst)) < 0)
        throw logger.errmsg("icmp_create, sendto");
}

raw_t icmp_socket(const char *ip, uint8_t sndtype) {
    __log;

    raw_t icmp;
    memset(&icmp, 0, sizeof(icmp));
    icmp.sndtype = sndtype;
    if ((icmp.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        throw logger.errmsg("raw socket");
    icmp.dst.sin_family = AF_INET;
    icmp.dst.sin_addr.s_addr = inet_addr(ip);
    return icmp;
}

uint32_t icmp_create(raw_t icmp, const char* dstname, unsigned short dstport) {
    __log;

    pkt_t pkt;
    uint32_t port = freeport++;


    mkpkt(pkt, ICMP_CREATE, port);
    pkt.load.ctl.dstport = dstport;
    strcpy((char*)pkt.load.ctl.dstname, dstname);

    icmp_sendto(icmp, pkt);

    return port;
}

void icmp_close(raw_t icmp, const uint32_t port) {
    __log;
    pkt_t pkt;
    mkpkt(pkt, ICMP_CLOSE, port);
    icmp_sendto(icmp, pkt);
}

void icmp_snd(raw_t icmp, uint32_t port, const void* buf, const size_t len) {
    __log;
    pkt_t pkt;
    assert(len <= ICMP_LEN);

    mkpkt(pkt, port, len, (const uint8_t*)buf);
    icmp_sendto(icmp, pkt);
}

void icmp_rcv(raw_t icmp,
        function<void(sockaddr_in, uint16_t, uint32_t, void*, size_t)> ircv,
        function<void(sockaddr_in, uint16_t, uint32_t, const char*, uint16_t)> iaccept,
        function<void(sockaddr_in, uint16_t, uint32_t)> iclose) {
}

