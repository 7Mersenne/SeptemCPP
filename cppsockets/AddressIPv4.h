#pragma once

#ifndef ADDRESS_IPV4_H
#define ADDRESS_IPV4_H

#include "Address.h"

namespace sockets {

class CAddressIPv4 : public IAddress {
protected:
	struct sockaddr_in _addr;
	bool _valid;
	
	
	static in_addr_t read_address(std::string hostname);
public:
	CAddressIPv4(){ _valid = false; }
	
	CAddressIPv4(std::string host, int port);
	CAddressIPv4(struct sockaddr_in addr);
	CAddressIPv4(const CAddressIPv4 &addr);
	
	virtual std::string str() const override;
	virtual bool Valid() const override;
	
	std::string ip() const;
	int port() const;
	
	struct sockaddr_in get() const { return _addr; }
};

}

#endif

