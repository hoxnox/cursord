/**@author $username$ <$usermail$>
 * @date $date$*/

#include <ipv4.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

TEST(IPv4, ParseCIDR)
{
	EXPECT_EQ(inet_addr("192.168.22.0"),    nx::CIDRtoIPv4Range("192.168.22.0/24").first);
	EXPECT_EQ(inet_addr("192.168.22.255"),  nx::CIDRtoIPv4Range("192.168.22.0/24").second);
	EXPECT_EQ(inet_addr("192.168.0.0"),     nx::CIDRtoIPv4Range("192.168.0.0/16").first);
	EXPECT_EQ(inet_addr("192.168.255.255"), nx::CIDRtoIPv4Range("192.168.0.0/16").second);
	EXPECT_EQ(inet_addr("192.0.0.0"),       nx::CIDRtoIPv4Range("192.0.0.0/8").first);
	EXPECT_EQ(inet_addr("192.255.255.255"), nx::CIDRtoIPv4Range("192.0.0.0/8").second);
	EXPECT_EQ(inet_addr("0.0.0.0"),         nx::CIDRtoIPv4Range("1.0.0.0/1").first);
	EXPECT_EQ(inet_addr("127.255.255.255"), nx::CIDRtoIPv4Range("1.0.0.0/1").second);
	EXPECT_EQ(inet_addr("127.0.0.1"),       nx::CIDRtoIPv4Range("127.0.0.1/32").first);
	EXPECT_EQ(inet_addr("127.0.0.1"),       nx::CIDRtoIPv4Range("127.0.0.1/32").second);
	EXPECT_EQ(0,                            nx::CIDRtoIPv4Range("0.0.0.0/1").first);
	EXPECT_EQ(0,                            nx::CIDRtoIPv4Range("0.0.0.0/1").first);
}

