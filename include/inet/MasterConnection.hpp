
#ifndef INET_MASTER_CONNECTION_HPP
#define INET_MASTER_CONNECTION_HPP

#include <map>
#include <vector>
#include "inet/TCPConnection.hpp"
#include "inet/UDPConnection.hpp"

namespace inet
{
	class MasterConnection
	{
		public:
			typedef std::function<bool (std::shared_ptr<IPConnection> const&)> processHandler;
			
			MasterConnection(void);
			~MasterConnection(void);
			bool isListening(void) const;
			unsigned long getNumConnections(void) const;
			unsigned int createMasterTCP(std::shared_ptr<processHandler> const& pAcceptPH, std::shared_ptr<processHandler> const& pChildPH);
			void removeMasterTCP(unsigned int connID);
			unsigned int createUDPConnection(std::shared_ptr<processHandler> const& pPH);
			void removeUDPConnection(unsigned int connID);
			void acceptConnection(unsigned int masterID, std::shared_ptr<TCPConnection> const& newTCPConnection);
			void stopListening(void);
			std::shared_ptr<TCPConnection> const answerIncomingConnection(void) const;

		private:
			std::thread listeningThread;
			std::mutex listeningThread_mutex;
			std::map<unsigned int, std::shared_ptr<IPConnection>> connections;
			mutable std::mutex conn_mutex;
			std::map<unsigned int, std::shared_ptr<processHandler>> processHandlers;
			mutable std::mutex proc_mutex;
			std::map<unsigned int, std::shared_ptr<processHandler>> masterChildProcessHandlers;
			mutable std::mutex mcproc_mutex;
			std::map<unsigned int, std::vector<unsigned int>> masterTCPList;
			mutable std::mutex masterTCPList_mutex;
			bool listening = false;
			mutable std::mutex listening_mutex;

			bool isListeningFinished(void) const;
			void setListeningState(bool state);
			void beginListening();
			void startListening();
			bool checkAllConnectionsForData(double timeout);
			int getLargestSocket(void) const;
			unsigned int addConnection(std::shared_ptr<IPConnection> const& pIPconn, std::shared_ptr<processHandler> const& pPH);
			void removeConnection(unsigned int connID);
			bool isConnMasterTCP(unsigned int connID) const;
	};
}

#endif /* INET_MASTER_CONNECTION_HPP */
