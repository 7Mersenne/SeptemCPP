#pragma once

#ifndef SOCKET_H
#define SOCKET_H

#include "SocketType.h"
#include <string>
#include <sstream>
#include <memory>

#include "Address.h"

#ifdef _DEBUG_
#include <iostream>
using namespace std;
#define LOG cerr << ">>> "
#endif

namespace sockets {
	using std::streambuf;
	using std::streamsize;
	using std::iostream;

	class CSocket;

	class CStreambufSocket : virtual public streambuf {
	private:
		int _buf;

		CStreambufSocket() {};

		int _uflow();
	protected:
		CSocket *_sock;

		virtual int sync() override;

		virtual streamsize showmanyc() override;

		virtual streamsize xsgetn(char *s, streamsize n) override;
		virtual int underflow() override;
		virtual int uflow() override;
		virtual int pbackfail(int c) override;

		virtual streamsize xsputn(const char *s, streamsize n) override;
		virtual int overflow(int c) override;
	public:
		CStreambufSocket(CSocket &sock);
		virtual ~CStreambufSocket() {}
	};

	class CSocket : public iostream {
	protected:
		CStreambufSocket _streambuf;
	public:
		CSocket() : iostream(&_streambuf), _streambuf(*this) {}
		virtual ~CSocket() {}

		virtual size_t recv(void *data, size_t count) = 0;
		virtual size_t send(const void *data, size_t count) = 0;

		virtual const IAddress &get_source_address() = 0;
		virtual const IAddress &get_dest_address() = 0;

		virtual bool Valid() = 0;

		virtual void Close() = 0;

		template<typename T>
		size_t recv(T &data) { return recv(&data, sizeof(T)); }
		template<typename T>
		size_t send(const T &data) { return send(&data, sizeof(T)); }
	};

	bool init_sockets();
	bool release_sockets();
}

#endif

