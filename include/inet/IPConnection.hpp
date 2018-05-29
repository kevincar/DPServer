
#ifndef INET_IP_CONNECTION_HPP
#define INET_IP_CONNECTION_HPP

#include <memory>
#include "inet/Socket.hpp"
#include "inet/ServiceAddress.hpp"

namespace inet
{
	class IPConnection
	{
		public:

			IPConnection(int type, int protocol);
			virtual ~IPConnection() = default;
			virtual bool send(void* data) const = 0;
		private:
			std::unique_ptr<Socket> socket;

		protected:
			std::unique_ptr<ServiceAddress> srcAddress;
			std::unique_ptr<ServiceAddress> destAddress;
	};
}

#endif /* INET_IP_CONNECTION_HPP */
