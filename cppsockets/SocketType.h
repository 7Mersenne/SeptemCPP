#pragma once

#ifndef SOCKET_TYPE_H
#define SOCKET_TYPE_H

#ifdef _WIN32
	#define WIN_SOCKET
#endif

#ifdef WIN_SOCKET
	#include <winsock.h>
#endif

namespace sockets {

#ifdef WIN_SOCKET
	typedef SOCKET socket_t;
	typedef unsigned int in_addr_t;
	typedef int socklen_t;
#else
	typedef int socket_t;
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
#endif

}

#endif

