/**@author Merder Kim <hoxnox@gmail.com>
 * @date 20131225 15:48:29
 * @copyright Merder Kim
 *
 * Simplest UDP messenger*/

#include <string>
#include <nx_socket.h>

/**@brief Simplest UDP messenger*/
class SimpleUDP
{
public:
	SimpleUDP(struct sockaddr* addr);
	size_t      Send(const std::string& data);
	std::string Recv();

private:
	SimpleUDP() {}; // disabled
	SOCKET    sock_;
	struct    sockaddr_storage addr_;
	socklen_t addrln_;
};

