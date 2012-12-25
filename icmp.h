#ifndef _ICMP_H
#define _ICMP_H

#include <cstdint>
#include <functional>

using std::function;

int icmp_socket();

/********************************************************
 * proxy client; snd
 * create a connection (|name| < 512, e.g. name="ptt.cc"
 ********************************************************/
uint32_t icmp_create(const char* name, unsigned short port);

/********************************************************
 * proxy client; snd
 * close a connection
 ********************************************************/
void icmp_close(const uint32_t port);

/********************************************************
 ********************************************************/
size_t icmp_snd(int icmp, uint32_t port, const void* buf, const size_t len);

/********************************************************
 ********************************************************/
void icmp_rcv(int icmp,
        function<void(uint32_t, void*, size_t)> ircv,
        function<void(uint32_t, const char*)> iaccept,
        function<void(uint32_t)> iclose);

#endif

