
#ifndef INET_TCP_CONNECTION_HPP
#define INET_TCP_CONNECTION_HPP

#include <thread>
#include "inet/IPConnection.hpp"

namespace inet
{
	class TCPConnection : public IPConnection
	{
		public:
			TCPConnection(void);
			TCPConnection(int captureRawSocket, sockaddr_in& captureAddr);
			bool send(void* data) const override;
	};
}

#endif /* INET_TCP_CONNECTION_HPP */
