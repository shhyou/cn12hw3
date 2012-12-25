#ifndef _SOCKET_H
#define _SOCKET_H

#ifndef EXTERN
#define EXTERN extern
#endif

#include <netdb.h>
#include <netinet/in.h>

#include <functional>

using std::function;

/* socket event handlers */
EXTERN function<void(int, sockaddr_in)> socke_accept;
EXTERN function<void(int)> socke_rcv;

void sock_init(); /* call this before you modify event handler */
void sock_final(); /* close all sockets */
void sock_listen(unsigned short port);
void sock_watch(int fd);
void sock_unwatch(int fd);
void sock_loop(); /* implement epoll here */
/* do: watch listen sockets (if any),
 * check if any socket ready to read
 * call socke_accept or socke_rcv         */

#endif

