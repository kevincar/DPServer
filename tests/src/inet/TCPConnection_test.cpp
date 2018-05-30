
#include "inet/TCPConnection.hpp"
#include "gtest/gtest.h"

TEST(TCPConnectionTest, Constructor)
{
	EXPECT_NO_THROW({
			inet::TCPConnection tcpc;
			});
}

TEST(TCPConnectionTest, setAddress)
{
	inet::TCPConnection tcpc;
	ASSERT_NO_THROW({
			tcpc.setAddress("0.0.0.0:0");
			});

}

TEST(TCPConnectionTest, listen)
{
	inet::TCPConnection tcpc;
	ASSERT_NO_THROW({
			tcpc.listen();
			})
}
