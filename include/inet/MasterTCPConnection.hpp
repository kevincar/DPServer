
#ifndef INET_MASTER_TCP_CONNECTION_HPP
#define INET_MASTER_TCP_CONNECTION_HPP

#include <vector>
#include "inet/TCPConnection.hpp"

namespace inet
{
	class MasterTCPConnection : public TCPConnection
	{
		public:
			typedef std::function<bool (std::shared_ptr<TCPConnection>&)> newConnectionAcceptHandlerFunc;
			typedef std::function<void (std::shared_ptr<TCPConnection>&)> connectionProcessHandlerFunc;
			
			MasterTCPConnection(void);
			~MasterTCPConnection(void);
			void listenForIncomingConnections(newConnectionAcceptHandlerFunc const& ncaHandler, connectionProcessHandlerFunc const& cpHandler);
			void stopListening(void);
			std::shared_ptr<TCPConnection> const answerIncomingConnection(void) const;
			void acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection);
		private:
			std::thread listeningThread;
			std::mutex listeningThread_mutex;
			std::vector<std::thread> connectionHandlerThreads;
			std::mutex chThreads_mutex;
			std::vector<std::shared_ptr<TCPConnection>> TCPConnections;
			std::mutex tcpc_mutex;
			bool listening = false;
			mutable std::mutex listening_mutex;
			newConnectionAcceptHandlerFunc newConnectionAcceptHandler;
			connectionProcessHandlerFunc connectionProcessHandler;

			bool isListening(void) const;
			bool isListeningFinished(void) const;
			void setListeningState(bool state);
			void beginListening();
	};
}

#endif /* INET_MASTER_TCP_CONNECTION_HPP */