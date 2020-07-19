
#ifndef NATHP_SERVER_HPP
#define NATHP_SERVER_HPP

#include "inet/MasterConnection.hpp"
#include "nathp/constants.hpp"
#include "nathp/Packet.hpp"

namespace nathp
{
	class Server
	{
		public:
			typedef std::function<bool(char const*, unsigned int)> ProcessHandler;

			enum State : unsigned char
			{
				OFF,
				STARTING,
				READY,
				BUSY
			};

			Server(inet::TCPAcceptor::AcceptHandler const& acceptHandler, ProcessHandler const& processHandler, unsigned int port = NATHP_PORT);

			std::vector<unsigned int> getClientList(void) const;
			bool connectoToClient(unsigned int clientId);
			State getState(void) const;

		private:
			bool internalAcceptHandler(inet::TCPConnection const& conn);
			bool internalProcessHandler(inet::TCPConnection const& conn);
			void processMessage(inet::TCPConnection const& connection, Packet const& packet);
			void setState(State s);

			std::unique_ptr<inet::MasterConnection> pMasterConnection;

			inet::TCPAcceptor::AcceptHandler externalAcceptHandler;
			ProcessHandler externalProcessHandler;

			unsigned int main_port;
			State state = State::OFF;

			mutable std::mutex state_mutex;
	};
}

#endif /* NATHP_SERVER_HPP */
