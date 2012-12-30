#include <cstring>
#include <cstdint>
#include <cassert>

#include <string>

#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h> // including everything in the universe
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>

#include <linux/ip.h>
#include <linux/icmp.h>

#include "icmp.h"
#include "log.h"

#define ICMP_CREATE 1
#define ICMP_CLOSE 2

#define ICMP_MAGIC 0x514B1E55  // tmt514's BlESS

#define ICMP_LEN 512

using std::string;

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

void mkpkt(pkt_t& pkt, const tunnel_t tnl, uint32_t len, const uint8_t data[ICMP_LEN]) {
    memset(&pkt, 0, sizeof(pkt));
    mkpkt_magic(pkt);
    pkt.port = tnl.port;
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

void icmp_sendto(raw_t icmp, const tunnel_t tnl, pkt_t& pkt) {
    __log;

    struct icmp_t {
        icmphdr hdr;
        pkt_t pkt;
    } data;

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = tnl.dst;

    data.hdr.type = icmp.sndtype;
    data.hdr.code = 0;
    data.hdr.checksum = 0;
    data.hdr.un.echo.id = tnl.echoid;
    data.hdr.un.echo.sequence = icmpseq++;

    //data.hdr.checksum = check((uint16_t*)&data.hdr, sizeof(data.hdr));

    data.pkt = pkt;

    if (sendto(icmp.fd, &data, sizeof(data), 0, (sockaddr*)&sin, sizeof(sin)) < 0)
        throw logger.errmsg("icmp_sendto");
}

void icmp_recvfrom(raw_t icmp, tunnel_t& tnlin, pkt_t &pkt) {
    __log;

    struct icmp_t {
        iphdr ip;
        icmphdr hdr;
        pkt_t pkt;
    } data;

    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    memset(&sin, 0, sizeof(sin));

    tnlin = tunnel_t();

    ssize_t rcv = recvfrom(icmp.fd, &data, sizeof(data), 0, (sockaddr*)&sin, &len);

    if (rcv < 0)
        throw logger.errmsg("recvfrom");

    if (data.hdr.type != (icmp.sndtype^8))
        logger.raise("not ping %s", (icmp.sndtype^8) == 0 ? "reply" : "request");

    if (icmp.sndtype == 8) { /* recv type is echo reply, i.e. sndtype is echo request */
        if (sin.sin_addr.s_addr != icmp.defdst)
            logger.raise("not my proxy");
        if (data.hdr.un.echo.id != icmp.defechoid)
            logger.raise("not my pid");
    }

    tnlin.echoid = data.hdr.un.echo.id;
    tnlin.dst = sin.sin_addr.s_addr;
    tnlin.port = data.pkt.port;
    pkt = data.pkt;

    logger.print("ping %s from %u.%u.%u.%u, echo id = %u, seq = %u",
            (icmp.sndtype^8) == 0 ? "reply" : "request",
            (tnlin.dst)&0xff, (tnlin.dst>>8)&0xff, (tnlin.dst>>16)&0xff, (tnlin.dst>>24)&0xff,
            (uint32_t)data.hdr.un.echo.id,
            (uint32_t)data.hdr.un.echo.sequence);
}

raw_t icmp_socket(const char *ip, uint8_t sndtype) {
    __log;

    raw_t icmp;
    memset(&icmp, 0, sizeof(icmp));
    icmp.sndtype = sndtype;
    if ((icmp.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        throw logger.errmsg("raw socket");
    icmp.defdst = inet_addr(ip);
    icmp.defechoid = getpid();
    return icmp;
}

tunnel_t icmp_create(raw_t icmp, const char* dstname, unsigned short dstport) {
    __log;

    tunnel_t tnl;
    pkt_t pkt;
    
    tnl.dst = icmp.defdst;
    tnl.echoid = icmp.defechoid;
    tnl.port = freeport;

    mkpkt(pkt, ICMP_CREATE, tnl.port);
    pkt.load.ctl.dstport = dstport;
    strcpy((char*)pkt.load.ctl.dstname, dstname);

    icmp_sendto(icmp, tnl, pkt);

    freeport++;
    return tnl;
}

void icmp_close(raw_t icmp, const tunnel_t tnl) {
    __log;
    pkt_t pkt;
    mkpkt(pkt, ICMP_CLOSE, tnl.port);
    icmp_sendto(icmp, tnl, pkt);
}

void icmp_snd(raw_t icmp, tunnel_t tnl, const void* buf, const size_t len) {
    __log;
    pkt_t pkt;
    assert(len <= ICMP_LEN);

    mkpkt(pkt, tnl, len, (const uint8_t*)buf);
    icmp_sendto(icmp, tnl, pkt);
}

void icmp_rcv(raw_t icmp,
        function<void(tunnel_t, void*, size_t)> ircv,
        function<void(tunnel_t, const char*, uint16_t)> iaccept,
        function<void(tunnel_t)> iclose) {
    __log;
    
    try {
        tunnel_t tnl;
        pkt_t pkt;

        icmp_recvfrom(icmp, tnl, pkt);
        unpkt_magic(pkt);

        if (pkt.port == 0) { /* control message */
            tnl.port = pkt.load.ctl.ctlport;
            if (pkt.load.ctl.type == ICMP_CREATE) {
                iaccept(tnl,
                        (const char*)pkt.load.ctl.dstname,
                        pkt.load.ctl.dstport);
            } else if (pkt.load.ctl.type == ICMP_CLOSE) {
                iclose(tnl);
            } else {
                logger.raise("unknown control message");
            }
        } else { /* normal message */
            ircv(tnl, pkt.load.msg.data, pkt.load.msg.len);
        }
    } catch (const string& e) {
        logger.print("Ignoring received icmp packet: %s", e.c_str());
    }
}

