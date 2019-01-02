
#include "gtest/gtest.h"
#include "inet/TCPAcceptor.hpp"

TEST(TCPAcceptorTest, constructor)
{
	ASSERT_NO_THROW({
			// Acceptor
			std::shared_ptr<inet::TCPAcceptor::AcceptHandler> acceptHandler = std::make_shared<inet::TCPAcceptor::AcceptHandler>([](std::shared_ptr<inet::TCPConnection> const& connection)->std::shared_ptr<inet::TCPConnection>{
					return std::shared_ptr<inet::TCPConnection> {nullptr};
					});

			// processor
			std::shared_ptr<inet::MasterConnection::processHandler> connectionHandler = std::make_shared<inet::MasterConnection::processHandler>([](std::shared_ptr<inet::IPConnection>)->bool{
					return true;
					});

			inet::TCPAcceptor tcpa (acceptHandler, connectionHandler);
			});
}

TEST(TCPAcceptorTest, getConnections)
{
	//Acceptor
	std::shared_ptr<inet::TCPAcceptor::AcceptHandler> acceptHandler = std::make_shared<inet::TCPAcceptor::AcceptHandler>([](std::shared_ptr<inet::TCPConnection> const& connection)->std::shared_ptr<inet::TCPConnection>{
			return std::shared_ptr<inet::TCPConnection> {nullptr};
			});

	// processor
	std::shared_ptr<inet::MasterConnection::processHandler> connectionHandler = std::make_shared<inet::MasterConnection::processHandler>([](std::shared_ptr<inet::IPConnection>)->bool{
			return true;
			});

	inet::TCPAcceptor tcpa (acceptHandler, connectionHandler);

	//std::shared_ptr<std::vector<std::shared_ptr<inet::TCPConnection const>>> pConnections = tcpa.getConnections();

	//size_t nConnections = pConnections->size();

	//ASSERT_EQ(nConnections, static_cast<size_t>(1));
}
