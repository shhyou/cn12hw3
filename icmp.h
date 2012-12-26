#ifndef _ICMP_H
#define _ICMP_H

#include <cstdint>
#include <cstring>
#include <functional>

#include <netinet/in.h>
#include <arpa/inet.h>

using std::function;

struct raw_t {
    int fd;
    uint8_t sndtype;
    uint16_t defechoid;
    in_addr_t defdst;
};

struct tunnel_t {
    uint16_t echoid;
    uint32_t port;
    in_addr_t dst;
    bool operator<(const tunnel_t& t) const { return memcmp(this, &t, sizeof(tunnel_t)) < 0; }
    tunnel_t() { memset(this, 0, sizeof(tunnel_t)); }
};

raw_t icmp_socket(const char *ip, uint8_t sndtype);

/********************************************************
 * proxy client; snd
 * create a connection (|name| < 512, e.g. name="ptt.cc"
 ********************************************************/
tunnel_t icmp_create(raw_t icmp, const char* name, unsigned short port);

/********************************************************
 * proxy client; snd
 * close a connection
 ********************************************************/
void icmp_close(raw_t icmp, const tunnel_t tunnel);

/********************************************************
 ********************************************************/
void icmp_snd(raw_t icmp, tunnel_t tunnel, const void* buf, const size_t len);

/********************************************************
 ********************************************************/
void icmp_rcv(raw_t icmp,
        function<void(tunnel_t, void*, size_t)> ircv,
        function<void(tunnel_t, const char*, uint16_t)> iaccept,
        function<void(tunnel_t)> iclose);

#endif

