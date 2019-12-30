#pragma once

#ifndef SOCKET_TCP_H
#define SOCKET_TCP_H

#include "AddressIPv4.h"
#include "Socket.h"

namespace sockets {
	class CSocketTCPServer;

	class CSocketTCP : public CSocket {
	protected:
		CAddressIPv4 _source, _dest;
		socket_t _socket;
		bool _valid;

		CSocketTCP() {}
		CSocketTCP(socket_t sock, struct sockaddr_in source, struct sockaddr_in dest);
		friend CSocketTCPServer;
	public:
		CSocketTCP(CAddressIPv4 dest);
		virtual ~CSocketTCP() override;

		virtual size_t recv(void *data, size_t count) override;
		virtual size_t send(const void *data, size_t count) override;

		virtual const IAddress &get_source_address() override { return _source; }
		virtual const IAddress &get_dest_address() override { return _dest; }

		virtual bool Valid() override;

		virtual void Open();
		virtual void Close() override;
	};

	class CSocketTCPServer : public CSocketTCP {
	protected:

	public:
		CSocketTCPServer(int port) : CSocketTCPServer(CAddressIPv4("0.0.0.0", port)) {}
		CSocketTCPServer(CAddressIPv4 source);

		virtual ~CSocketTCPServer() override;

		virtual size_t recv(void *data, size_t count) override { return 0; }
		virtual size_t send(const void *data, size_t count) override { return 0; }

		virtual bool Valid() override;

		virtual void Open() override { Listen(1); }

		void Listen(int cnt);
		std::unique_ptr<CSocketTCP> accept();
	};

}

#endif

