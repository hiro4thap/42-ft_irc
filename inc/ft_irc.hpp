/*
The reference client for this project is KVIrc v5.0.0 for MacOX.
It can be downloaded here: https://www.kvirc.net/?id=releases&platform=macosx&version=5.0.0&lang=en
*/

#ifndef FT_IRC_HPP
# define FT_IRC_HPP

// socket, setsockopt, getsockname, bind, connect, listen, accept, send, recv
// getprotobyname, gethostbyname, 
// gethostbyname, getaddrifno, freeaddrinfo, 
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>

// htons, htonl, ntohs, ntohl
// inet_addr, inet_ntoa
# include <arpa/inet.h>
// # include <inet/in.h>

// signal, sigaction
# include <signal.h>

// poll / select / kqueu / epoll
# include <poll.h>
// # include <sys/select.h>
// # include <sys/event.h>
// # include <sys/epoll.h>

// lseek
# include <unistd.h>

// fstat
# include <sys/stat.h>

// fcntl
# include <fcntl.h>

#include <cstdio>

# include "Log.hpp"
# include "Parser.hpp"

#define SHOW_SERVER_MSG true

#endif
