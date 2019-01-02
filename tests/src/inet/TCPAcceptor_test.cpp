
#include "gtest/gtest.h"
#include <g3log/g3log.hpp>
#include "inet/TCPAcceptor.hpp"
#include "inet/TCPAcceptor_test.hpp"

// ==================================================================

std::unique_ptr<std::thread> startTestServer(std::string& serverAddress, std::mutex& serverAddressMutex, std::string& status, std::mutex& statusMutex, std::condition_variable& statusCV)
{
	//std::cout << "Server: Starting thread..." << std::endl;
	LOG(DEBUG) << "Server: Starting thread...";
	return std::make_unique<std::thread>([&]
	{
		bool done = false;
		inet::TCPAcceptor::AcceptHandler acceptHandler = [](inet::TCPConnection const& connection)->bool
		{
			if(connection)
			{
				std::cout << "Server: new connection from: " << connection.getAddressString() << std::endl;
			}
			return true;
		};
	
		inet::TCPAcceptor::ProcessHandler processHandler = [](inet::IPConnection const& connection)->bool
		{
			if(connection)
			{
				int const bufsize = 1024*4;
				char buffer[bufsize] {};
				int result = connection.recv(buffer, bufsize);
				EXPECT_NE(result, -1);
				if(result == 0)
				{
					return false;
				}

				std::cout << "Server: Received data: " << std::string(buffer) << std::endl;
			}
			return true;
		};

		// Start up the server
		std::cout << "Server: initializing..." << std::endl;
		inet::TCPAcceptor tcpa(acceptHandler, processHandler);
		std::cout << "Server: main listening fd is " << static_cast<int>(tcpa) << std::endl;
		tcpa.setAddress("0.0.0.0:0");
		{
			std::lock_guard<std::mutex> serverAddressLock {serverAddressMutex};
			serverAddress = tcpa.getAddressString();
			tcpa.listen();
			std::cout << "Server: server started with address as " << serverAddress << std::endl;
		}

		// This proccessor thread is responsible for checking for incoming data
		std::cout << "Server: starting processor thread..." << std::endl;
		std::thread processor([&]
		{
			// continue while not done
			bool isServerDone = false;
			while(!isServerDone)
			{
				fd_set fdSet;
				FD_ZERO(&fdSet);

				// load the fd_set with connections
				tcpa.loadFdSetConnections(fdSet);
				//if(tcpa.isDataReady(2))
				//{
					//std::cout << "hot dog" << std::endl;
				//}

				// call select
				int const largestSocket = tcpa.getLargestSocket();
				//std::cout << "Server: largest socket = " << largestSocket << std::endl;
				struct timeval tv;
				tv.tv_sec = 10;
				tv.tv_usec = 0;
				int result = ::select(largestSocket + 1, &fdSet, NULL, NULL, &tv);
				ASSERT_NE(result, -1);

				// check connections
				tcpa.checkAndProcessConnections(fdSet);
				//isServerDone = true;
				//std::cout << "Server: completed..." << std::endl;
			}
		});

		processor.join();

		std::unique_lock<std::mutex> statusLock {statusMutex};
		statusCV.wait(statusLock, [&]{return status == "done";});

		return;
	});
}

std::unique_ptr<std::thread> startTestClient(std::string& serverAddress, std::mutex& serverAddressMutex, std::string& status, std::mutex& statusMutex, std::condition_variable& statusCV)
{
	return std::make_unique<std::thread>([&]
	{
		// Start the client
		bool connected = false;
		std::cout << "Client: initializing client..." << std::endl;
		inet::TCPConnection tcpc{};
		std::cout << "Client: client fd = " << static_cast<unsigned>(tcpc) << std::endl;
		while(!connected)
		{
			std::lock_guard<std::mutex> serverAddressLock {serverAddressMutex};
			std::cout << "Client: Attempting to connect to " << serverAddress << std::endl;
			int result = tcpc.connect(serverAddress);
			if(result == 0)
			{
				connected = true;
			}
		}

		// Send Data
		std::string sendData = "Test Data.";
		int result = tcpc.send(sendData.c_str(), static_cast<unsigned>(sendData.size()+1));
		std::cout << "Client: test data sent." << std::endl;
		EXPECT_NE(result, -1);

		return;
	});
}

// ==================================================================

TEST(TCPAcceptorTest, constructor)
{
	ASSERT_NO_THROW({
			// Acceptor
			inet::TCPAcceptor::AcceptHandler acceptHandler = [](inet::TCPConnection const& connection)->bool{
			if(connection) return true;
			return true;
			};

			// processor
			inet::TCPAcceptor::ProcessHandler connectionHandler = [](inet::IPConnection const& connection)->bool{
			if(connection) return true;
			return true;
			};

			inet::TCPAcceptor tcpa (acceptHandler, connectionHandler);
			});
}

TEST(TCPAcceptorTest, getLargestSocket)
{
	// Acceptor
	inet::TCPAcceptor::AcceptHandler acceptHandler = [](inet::TCPConnection const& connection)->bool{
		if(connection) return true;
		return true;
	};

	// processor
	inet::TCPAcceptor::ProcessHandler connectionHandler = [](inet::IPConnection const& connection)->bool{
		if(connection) return true;
		return true;
	};

	inet::TCPAcceptor tcpa {acceptHandler, connectionHandler};
	int largestSocket = -1;

	largestSocket = tcpa.getLargestSocket();

	EXPECT_NE(largestSocket, -1);
}

TEST(TCPAcceptorTest, getConnections)
{
	//Acceptor
	inet::TCPAcceptor::AcceptHandler acceptHandler = [](inet::TCPConnection const& connection)->bool{
			if(connection) return true;
			return true;
			};

	// processor
	inet::TCPAcceptor::ProcessHandler connectionHandler = [](inet::IPConnection const& connection)->bool{
			if(connection) return true;
			return true;
			};

	//inet::TCPAcceptor tcpa (acceptHandler, connectionHandler);
	std::shared_ptr<inet::TCPAcceptor> tcpa = std::make_shared<inet::TCPAcceptor>(acceptHandler, connectionHandler);

	//std::shared_ptr<std::vector<inet::TCPConnection const*>> pConnections = tcpa->getConnections();
	std::vector<inet::TCPConnection const*> const connections = tcpa->getConnections();

	size_t nConnections = connections.size();

	ASSERT_EQ(nConnections, static_cast<size_t>(0));
}

TEST(TCPAcceptorTest, accpetProcessAndRemove)
{
	unsigned int acceptHandlerCount = 0;
	unsigned int processHandlerCount = 0;
	std::mutex addressMutex;
	std::string serverAddress{};
	std::mutex statusMutex;
	std::string status{};
	std::condition_variable statusCV;
	std::mutex doneMutex;
	bool processDone = false;

	// AcceptHandler
	inet::TCPAcceptor::AcceptHandler acceptHandler = [&](inet::TCPConnection const& connection) -> bool
	{
		if(connection){}
		acceptHandlerCount++;
		//std::cout << "Server: Connection accepted" << std::endl;
		return true;
	};

	// Process Handler Template
	inet::TCPAcceptor::ProcessHandler processHandler = [&](inet::TCPConnection const& connection) -> bool
	{
		int const bufsize = 1024*4;
		char buffer[bufsize] {};
		int result = connection.recv(buffer, bufsize);
		EXPECT_NE(result, -1);
		if(result == 0)
		{
			//std::cout << "Server: Client connection closed." << std::endl;
			return false;
		}
		std::string data {buffer};

		if(connection){}
		processHandlerCount++;

		//std::cout << "Data: " << data << std::endl;

		data = "Thank you.";
		result = connection.send(data.c_str(), static_cast<unsigned int>(data.size()+1));
		EXPECT_GE(result, 1);

		return true;
	};

	std::thread server([&]{
			std::unique_lock<std::mutex> statusLock {statusMutex, std::defer_lock};

			// Set up server
			//std::cout << "Server: Starting..." << std::endl;
			inet::TCPAcceptor tcpa{acceptHandler, processHandler};
			tcpa.setAddress("0.0.0.0:0");
			{
				std::lock_guard<std::mutex> addressLock{addressMutex};
				serverAddress = tcpa.getAddressString();
				tcpa.listen();
				//std::cout << "Server: set address to " << serverAddress << std::endl;
			}

			//std::cout << "Server: Starting acceptor..." << std::endl;
			std::thread acceptor([&]{
				bool isServerDone = false;
				while(!isServerDone)
				{
					{
						std::lock_guard<std::mutex> doneLock {doneMutex};
						isServerDone = processDone;
					}

					if(isServerDone) continue;

					//std::cout << "Acceptor: Checking for new connections" << std::endl;
					if(tcpa.isDataReady(0.25))
					{
						//std::cout << "Acceptor: New connection incoming" << std::endl;
						tcpa.accept();
						ASSERT_EQ(acceptHandlerCount, static_cast<unsigned>(1));
					}
				}
			});
			//std::cout << "Server: acceptor started" << std::endl;

			//std::cout << "Server: starting connection processor..." << std::endl;
			std::thread connectionProcessing([&]
			{
				bool isServerDone = false;
				while(!isServerDone)
				{
					{
						std::lock_guard<std::mutex> doneLock {doneMutex};
						isServerDone = processDone;
					}

					if(isServerDone) continue;

					std::vector<inet::TCPConnection const*> connections = tcpa.getConnections();
					for(unsigned long i = 0; i < connections.size(); i++)
					{
						//std::cout << "i: " << i << std::endl;
						inet::TCPConnection const* curConn = connections.at(i);
						//std::cout << "Proc: Checking connection for data" << std::endl;
						if(curConn->isDataReady(1))
						{
							//std::cout << "Proc: connection has data" << std::endl;
							bool keepConnection = tcpa.getConnectionHandler()(*curConn);
							ASSERT_LE(processHandlerCount, static_cast<unsigned>(1));

							if(!keepConnection)
							{
								// Remove Connection
								tcpa.removeConnection(*curConn);
								ASSERT_EQ(tcpa.getConnections().size(), static_cast<unsigned long>(0));
								//std::cout << "Proc: Connection removed" << std::endl;

								if(connections.size() == 1)
								{
									std::lock_guard<std::mutex> doneLock {doneMutex};
									processDone = true;
								}
							}
						}
					}
				}
			});
			//std::cout << "Server: Proc Started" << std::endl;
			
			statusLock.lock();
			status = "Server Started.";
			statusLock.unlock();
			statusCV.notify_one();

			statusLock.lock();
			statusCV.wait(statusLock, [&]{return status == "Client Complete.";});
			
			acceptor.join();
			connectionProcessing.join();

			});

	std::thread client([&]{
			//std::cout << "Client: starting..." << std::endl;
			inet::TCPConnection tcp{};
			std::unique_lock<std::mutex> statusLock {statusMutex};
			//std::cout << "Client: Waiting for server to start" << std::endl;
			statusCV.wait(statusLock, [&]{return status == "Server Started.";});
			statusLock.unlock();

			{
				std::lock_guard<std::mutex> addressLock {addressMutex};
				//std::cout << "Client: connecting to " << serverAddress << std::endl;
				int result = tcp.connect(serverAddress);
				//std::cout << "Client: connected" << std::endl;
				EXPECT_EQ(result, 0);
			}

			std::string sendData = "Test Data.";
			int result = tcp.send(sendData.c_str(), static_cast<unsigned>(sendData.size()+1));
			//std::cout << "sent data" << std::endl;
			EXPECT_NE(result, -1);

			const unsigned int bufSize = 255;
			char buffer[bufSize];

			tcp.recv(buffer, bufSize);
			std::string data {buffer};

			ASSERT_STREQ(data.data(), "Thank you.");

			statusLock.lock();
			status = "Client Complete.";
			statusLock.unlock();
			statusCV.notify_one();
			});
	
	server.join();
	client.join();
}

TEST(TCPAcceptorTest, loadFdSetConnections)
{
	// Setup handlers
	inet::TCPAcceptor::AcceptHandler acceptHandler = [&](inet::TCPConnection const& connection) -> bool {return true;};
	inet::TCPAcceptor::ProcessHandler processHandler = [&](inet::TCPConnection const& connection) -> bool {return true;};

	inet::TCPAcceptor tcpa(acceptHandler, processHandler);

	fd_set fdSet;
	FD_ZERO(&fdSet);
	tcpa.loadFdSetConnections(fdSet);
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500;
	int result = ::select(tcpa.getLargestSocket()+1, &fdSet, NULL, NULL, &tv);
	ASSERT_NE(result, -1);
}

TEST(TCPAcceptorTest, checkAndProcessConnections)
{
	std::string status;
	std::mutex statusMutex;
	std::condition_variable statusCV;

	// Server
	std::string serverAddress {"0.0.0.0:0"};
	std::mutex serverAddressMutex;
	std::unique_ptr<std::thread> serverThread = startTestServer(serverAddress, serverAddressMutex, status, statusMutex, statusCV);

	// Client
	std::unique_ptr<std::thread> clientThread = startTestClient(serverAddress, serverAddressMutex, status, statusMutex, statusCV);
	
	serverThread->join();
	clientThread->join();
}
