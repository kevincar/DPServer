#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "inet/MasterTCPConnection.hpp"

namespace inet
{
	MasterTCPConnection::MasterTCPConnection(void) : TCPConnection() {}

	MasterTCPConnection::~MasterTCPConnection(void)
	{
		if(this->isListening())
		{
			this->stopListening();
		}
	}

	void MasterTCPConnection::listenForIncomingConnections(MasterTCPConnection::newConnectionAcceptHandlerFunc const& ncah, MasterTCPConnection::connectionProcessHandlerFunc const& cph)
	{
		if(this->isListening()) return;
		this->listen();
		this->setListeningState(true);
		std::lock_guard<std::mutex> lock {this->listeningThread_mutex};
		this->newConnectionAcceptHandler = ncah;
		this->connectionProcessHandler = cph;
		this->listeningThread = std::thread([=]{this->beginListening();});
	}

	void MasterTCPConnection::stopListening(void)
	{
		if(std::this_thread::get_id() == this->listeningThread.get_id())
		{
			this->setListeningState(false);
		}
		else
		{
			this->setListeningState(false);
			//std::unique_lock<std::mutex> lock {this->listeningFinished_mutex};
			//this->listeningFinished_cv.wait(lock, [=]{return this->isListeningFinished();});
			this->listeningThread.join();
		}
	}

	std::shared_ptr<TCPConnection> const MasterTCPConnection::answerIncomingConnection(void) const
	{
		sockaddr_in addr {};
		unsigned int addrsize = sizeof(sockaddr_in);

		int newSocket = ::accept(*this->socket.get(), (sockaddr*)&addr, &addrsize);
		if(newSocket == -1)
		{
			std::cout << "MasterTCPConnection::listen Failed to accept incoming connection " << errno << std::endl;
		}

		// Assemble the data into a new TCPConnection object
		return std::make_shared<TCPConnection>(newSocket, static_cast<TCPConnection const&>(*this), addr);
	}

	void MasterTCPConnection::acceptConnection(std::shared_ptr<TCPConnection>& newTCPConnection)
	{
		// Add the connection to our list of connections
		{
			//std::lock_guard<std::mutex> lock {this->tcpc_mutex};
			//this->TCPConnections.emplace_back(newTCPConnection);
		}

		// Start the connection process thread
		{
			//std::lock_guard<std::mutex> lock {this->chThreads_mutex};
			//this->connectionHandlerThreads.emplace_back(std::thread([this, &newTCPConnection]{this->connectionProcessHandler(newTCPConnection);}));
		}
	}

	bool MasterTCPConnection::isListening(void) const
	{
		std::lock_guard<std::mutex> lock {this->listening_mutex};
		return this->listening;
	}

	void MasterTCPConnection::setListeningState(bool state)
	{
		std::lock_guard<std::mutex> lock {this->listening_mutex};
		this->listening = state;
	}

	void MasterTCPConnection::beginListening()
	{
		// Check for a new connection every 5 seconds
		while(this->isListening())
		{
			if(this->isDataReady(5.0)) {
					}
		}
	}

	bool MasterTCPConnection::checkAllConnectionsForData(double timeout)
	{
		fd_set fdSet;
		struct timeval tv;
		int largestFD = this->getLargestSocket();
		bool result = false;

		// Clear the set
		FD_ZERO(&fdSet);

		// Add all sockets to the set
		FD_SET(*this, &fdSet);
		for(std::shared_ptr<TCPConnection> pCurConn : this->TCPConnections)
		{
			FD_SET(*pCurConn, &fdSet);
		}

		// Set timeout
		int seconds = static_cast<int>(floor(timeout));
		double remainder = timeout - seconds;
		double remainder_us = remainder * 1e6;
		int microseconds = static_cast<int>(floor(remainder_us));

		tv.tv_sec = seconds;
		tv.tv_usec = microseconds;

		int retval = select(largestFD+1, &fdSet, nullptr, nullptr, &tv);

		if(retval == -1) {
			throw "MasterTCPConnection::checkAllConnectionsForData - failed to select!";
		}

		if(FD_ISSET(*this, &fdSet) == true)
		{
			result = true;
			std::shared_ptr<TCPConnection> newConnection = this->answerIncomingConnection();
			bool acceptConnection = this->newConnectionAcceptHandler(newConnection);
			if(acceptConnection)
			{
				this->acceptConnection(newConnection);
			}
		}

		for(std::shared_ptr<TCPConnection> pCurConn : this->TCPConnections)
		{
			if(FD_ISSET(*pCurConn, &fdSet) == true)
			{
				result = true;
				this->connectionProcessHandler(pCurConn);
			}
		}

		return result;
	}

	int MasterTCPConnection::getLargestSocket(void) const
	{
		int currentSocket = *this;
		int result = currentSocket;

		for(std::shared_ptr<TCPConnection> pCurConn : this->TCPConnections)
		{
			 currentSocket = *pCurConn;
			 if(currentSocket > result)
				 result = currentSocket;
		}

		return result;
	}
}
