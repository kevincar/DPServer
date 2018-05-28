
#include "inet/ServiceAddress.hpp"
#include "gtest/gtest.h"

TEST(ServiceAddress, constructor)
{
	EXPECT_NO_THROW({
			inet::ServiceAddress addr("192.168.1.1:2300");
			});
}

TEST(ServiceAddress, badconstructor)
{
	EXPECT_ANY_THROW({
			inet::ServiceAddress addr("19592.2");
			});
}

TEST(ServiceAddress, getAddressString)
{
	inet::ServiceAddress addr("192.168.1.100:47624");
	ASSERT_STREQ(addr.getAddressString().data(), "192.168.1.100");
}

TEST(ServiceAddress, getPortString)
{
	inet::ServiceAddress addr("10.0.0.1:2400");
	ASSERT_STREQ(addr.getPortString().data(), "2400");
}
