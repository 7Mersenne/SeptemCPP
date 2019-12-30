#include "AddressIPv4.h"
#include <sstream>

#ifndef WIN_SOCKET
	#include <netdb.h>
#endif

namespace sockets {

in_addr_t CAddressIPv4::read_address(std::string hostname){
	struct hostent *h = gethostbyname(hostname.c_str());
	if (!h){
		return 0;
	}
	return *((unsigned int *) h->h_addr_list[0]);
}

CAddressIPv4::CAddressIPv4(std::string host, int port){
	struct hostent *h = gethostbyname(host.c_str());
	_valid = h != nullptr;
	if (_valid){
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons((uint16_t)port);
		_addr.sin_addr.s_addr = *((unsigned int *) h->h_addr_list[0]); //htonl(INADDR_ANY);
	}
}

CAddressIPv4::CAddressIPv4(struct sockaddr_in addr){
	_addr = addr;
	_valid = true;
}

CAddressIPv4::CAddressIPv4(const CAddressIPv4 &addr){
	_addr = addr._addr;
	_valid = addr._valid;
}

std::string CAddressIPv4::str() const {
	std::stringstream res;
	res << ip() << ":" << port();
	return res.str();
}

bool CAddressIPv4::Valid() const {
	return _valid;
}

std::string CAddressIPv4::ip() const {
	std::stringstream res;
	in_addr_t a = ntohl(_addr.sin_addr.s_addr);
	for (int i = 0; i < 4; ++i, a <<= 8){
		res << ((a & 0xff000000) >> 24);
		if (i < 3){
			res << ".";
		}
	}
	return res.str();
}

int CAddressIPv4::port() const {
	return ntohs(_addr.sin_port);
}

}

