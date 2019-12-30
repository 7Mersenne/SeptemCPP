#pragma once

#ifndef ADDRESS_H
#define ADDRESS_H

#include "SocketType.h"
#include <string>

#ifndef WIN_SOCKET
	#include <netinet/in.h>
#else
	#include <winsock.h>
#endif

namespace sockets {

class IAddress {
public:
	virtual std::string str() const = 0;
	virtual bool Valid() const = 0;
};

}

#endif

