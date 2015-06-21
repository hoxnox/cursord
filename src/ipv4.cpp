/**@author $username$ <$usermail$>
 * @date $date$ */

#include <ipv4.hpp>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace nx {

std::pair<uint32_t, uint32_t> CIDRtoIPv4Range(std::string str)
{
	size_t pos = str.find('/');
	if (pos > 15)
		return std::make_pair(0,0);
	if (pos == std::string::npos)
		return std::make_pair(0,0);
	uint32_t addr = inet_addr(str.substr(0, pos).c_str());
	if (addr == INADDR_NONE)
		return std::make_pair(0,0);
	int mask = atoi(str.substr(pos + 1, str.length() - pos - 1).c_str());
	if (mask > 32 || mask <= 0)
		return std::make_pair(0,0);
	if (mask == 32)
		return std::make_pair(addr, addr);
	uint32_t haddr = ntohl(addr);
	uint32_t ip_low = haddr & ((0xffffffff>>(32-mask))<<(32-mask));
	uint32_t ip_hi = ip_low | (0xffffffff>>mask);
	return std::make_pair(htonl(ip_low), htonl(ip_hi));
}

} // namespace
