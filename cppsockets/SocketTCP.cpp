#ifndef WIN_SOCKET
	#include <unistd.h>
#endif

#include "SocketTCP.h"

namespace sockets {

CSocketTCP::CSocketTCP(socket_t sock, struct sockaddr_in source, struct sockaddr_in dest){
	_socket = sock;
	_source = CAddressIPv4(source);
	_dest = CAddressIPv4(dest);
	_valid = true;
}

CSocketTCP::CSocketTCP(CAddressIPv4 dest){
	_socket = INVALID_SOCKET;
	_dest = dest;
	_valid = false;
}

CSocketTCP::~CSocketTCP(){
	Close();
}

size_t CSocketTCP::recv(void *data, size_t count){
	if (Valid()){
		size_t l = 0;
		while (l < count){
			int k = (int)::recv(_socket, ((char *) data + l), count - l, 0);
			if (k <= 0){
				_valid = false;
				return l;
			}
			l += k;
		}
		return l;
	}
	return 0;
}

size_t CSocketTCP::send(const void *data, size_t count){
	if (Valid()){
		size_t l = 0;
		while (l < count){
			int k = (int)::send(_socket, ((const char *) data + l), count - l, 0);
			if (k <= 0){
				_valid = false;
				return l;
			}
			l += k;
		}
		return l;
	}
	return 0;
}

bool CSocketTCP::Valid(){
	return _source.Valid() && _dest.Valid() && _socket != INVALID_SOCKET && _valid;
}

void CSocketTCP::Open(){
	if (_dest.Valid()){
		_socket = ::socket(AF_INET, SOCK_STREAM, 0);
		if (_socket == INVALID_SOCKET){
			return;
		}
		
		auto addr = _dest.get();
		socklen_t len = sizeof(addr);
		if (::connect(_socket, (struct sockaddr *) &addr, len) == SOCKET_ERROR){
		    Close();
		    return;
		}
		
		if (::getsockname(_socket, (struct sockaddr *) &addr, &len) == SOCKET_ERROR || len != sizeof(addr)){
		    Close();
		    return;
		}
		
		_source = addr;
		_valid = true;
	}
}

void CSocketTCP::Close(){
	if (INVALID_SOCKET){
#ifdef WIN_SOCKET
		::closesocket(_socket);
#else
		::close(_socket);
#endif
		_socket = SOCKET_ERROR;
		_valid = false;
	}
}

// --- socket_tcp_server ---

CSocketTCPServer::CSocketTCPServer(CAddressIPv4 source)
		: CSocketTCP()
{
	_source = source;
}

CSocketTCPServer::~CSocketTCPServer(){
	
}

bool CSocketTCPServer::Valid(){
	return _source.Valid() && _socket != INVALID_SOCKET && _valid;
}

void CSocketTCPServer::Listen(int cnt){
	if (_source.Valid()){
		_socket = ::socket(AF_INET, SOCK_STREAM, 0);
		if (_socket == INVALID_SOCKET){
			return;
		}

		char val = 1;
		if (::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == SOCKET_ERROR){
			Close();
			return;
		}
		
		auto addr = _source.get();
		if (::bind(_socket, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR){
			Close();
			return;
		}
	
		::listen(_socket, cnt);
		_valid = true;
	}
}

std::unique_ptr<CSocketTCP> CSocketTCPServer::accept(){
	if (Valid()){
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		socket_t sock = ::accept(_socket, (struct sockaddr *) &addr, &addrlen);
		CSocketTCP *s = new CSocketTCP(sock, addr, _source.get());
		return std::unique_ptr<CSocketTCP>(s);
	}
	return std::unique_ptr<CSocketTCP>(nullptr);
}

}

