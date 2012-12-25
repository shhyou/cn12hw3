#ifndef _ICMP_H
#define _ICMP_H

#include <cstdint>
#include <functional>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

using std::function;

struct raw_t {
    int fd;
    uint8_t sndtype;
    sockaddr_in dst;
};

raw_t icmp_socket(const char *ip, uint8_t sndtype);

/********************************************************
 * proxy client; snd
 * create a connection (|name| < 512, e.g. name="ptt.cc"
 ********************************************************/
uint32_t icmp_create(raw_t icmp, const char* name, unsigned short port);

/********************************************************
 * proxy client; snd
 * close a connection
 ********************************************************/
void icmp_close(raw_t icmp, const uint32_t port);

/********************************************************
 ********************************************************/
void icmp_snd(raw_t icmp, uint32_t port, const void* buf, const size_t len);

/********************************************************
 ********************************************************/
void icmp_rcv(raw_t icmp,
        function<void(sockaddr_in, uint16_t, uint32_t, void*, size_t)> ircv,
        function<void(sockaddr_in, uint16_t, uint32_t, const char*, uint16_t)> iaccept,
        function<void(sockaddr_in, uint16_t, uint32_t)> iclose);

#endif

