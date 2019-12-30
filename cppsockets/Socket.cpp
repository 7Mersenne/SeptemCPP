#include "Socket.h"

#include <stdio.h>

namespace sockets {

#ifdef WIN_SOCKET
	static WSADATA w_wsa_data;
#endif

bool init_sockets(){
#ifdef WIN_SOCKET
	WSAStartup(MAKEWORD(2, 2), &w_wsa_data);
#endif
	return true;
}

bool release_sockets(){
#ifdef WIN_SOCKET
	WSACleanup();
#endif
	return true;
}

// streambuf_socket //

CStreambufSocket::CStreambufSocket(CSocket &sock){
	_sock = &sock;
	_buf = EOF;
}

int CStreambufSocket::sync(){
#ifdef _DEBUG_
		LOG << __FUNCTION__ << endl;
#endif
		return 0;
}

streamsize CStreambufSocket::showmanyc(){
#ifdef _DEBUG_
	LOG << __FUNCTION__ << endl;
#endif
	return _buf != EOF ? 1 : 0;
}

streamsize CStreambufSocket::xsgetn(char *s, streamsize n){
#ifdef _DEBUG_
	LOG << __FUNCTION__ << endl;
#endif
	streamsize sz = _sock->recv(s, n);
	_buf = EOF;
	return sz;
}

int CStreambufSocket::_uflow(){
#ifdef _DEBUG_
	LOG << __FUNCTION__ << endl;
#endif
	char c;
	if (_sock->recv(&c, sizeof(c)) == sizeof(c)){
		_buf = c;
	} else {
		_buf = EOF;
	}
	
	return _buf;
}

int CStreambufSocket::underflow(){
#ifdef _DEBUG_
	LOG << __FUNCTION__ << endl;
#endif
	if (_buf == EOF){
		return _uflow();
	}
	return _buf;
}

int CStreambufSocket::uflow(){
#ifdef _DEBUG_
		LOG << __FUNCTION__ << endl;
#endif
	int c = _buf;
	if (c == EOF){
		c = _uflow();
		if (c == EOF){
			return EOF;
		}
	}
	
	_buf = EOF;
	
	return c;
}

int CStreambufSocket::pbackfail(int c){
#ifdef _DEBUG_
	LOG << __FUNCTION__ << endl;
#endif
	return EOF;
}

streamsize CStreambufSocket::xsputn(const char *s, streamsize n){
#ifdef _DEBUG_
	LOG << __FUNCTION__ << endl;
#endif
	return _sock->send(s, n);
}

int CStreambufSocket::overflow(int c){
#ifdef _DEBUG_
	LOG << __FUNCTION__ << endl;
#endif
	if (c != EOF && _sock->send((char) c) == sizeof(char)){
		return c;
	}
	return EOF;
}

}

