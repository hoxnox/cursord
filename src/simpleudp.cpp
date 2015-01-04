/**@author Merder Kim <hoxnox@gmail.com>
 * @date 20131225 15:48:29
 * @copyright Merder Kim*/

#include <simpleudp.hpp>
#include <gettext.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

SimpleUDP::SimpleUDP(struct sockaddr* addr)
	: sock_(INVALID_SOCKET)
{
	sock_ = socket(addr->sa_family, SOCK_DGRAM, 0);
	if (!IS_VALID_SOCK(sock_))
	{
		std::cerr << _("Error socket initialization.") << " "
		          << _(" Message: ")<< strerror(GET_LAST_SOCK_ERROR())
		          << std::endl;
		sock_ = INVALID_SOCKET;
		return;
	}
	if (SetNonBlock(sock_) < 0)
	{
		std::cerr << _("Error switching socket to non-block mode.") << " "
		          << _(" Message: ")<< strerror(GET_LAST_SOCK_ERROR())
		          << std::endl;
		close(sock_);
		sock_ = INVALID_SOCKET;
		return;
	}
	if (SetReusable(sock_) < 0)
	{
		std::cerr << _("Error making socket reusable") << " "
		          << _(" Message: ")<< strerror(GET_LAST_SOCK_ERROR())
		          << std::endl;
		close(sock_);
		sock_ = INVALID_SOCKET;
		return;
	}
	CopySockaddrToStorage(addr, &addr_);
	addrln_ = sizeof(struct sockaddr_storage);
}

size_t
SimpleUDP::Send(const std::string& data)
{
	if (sock_ == INVALID_SOCKET)
	{
		std::cerr << _("Error sending data.")
		          << _(" Message: Socket is invalid")
		          << std::endl;
		return 0;
	}
	int rs = sendto(sock_, data.c_str(), data.length(), 0,
			(struct sockaddr*)&addr_, addrln_);
	if( rs < 0 )
	{
		std::cerr << _("Error sending command.") << " "
		          << _(" Message: ")<< strerror(GET_LAST_SOCK_ERROR())
		          << std::endl;
		return -1;
	}
}

std::string
SimpleUDP::Recv()
{
	std::string result;
	if (sock_ == INVALID_SOCKET)
	{
		std::cerr << _("Error receiving data.")
		          << _(" Message: Socket is invalid")
		          << std::endl;
		return result;
	}

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sock_, &rfds);
	int rs = select(sock_ + 1, &rfds, NULL, NULL, &timeout);
	if(rs < 0)
	{
		std::cerr << _("Error receiving data.")
		          << _(" Message: ")<< strerror(GET_LAST_SOCK_ERROR())
		          << std::endl;
		return result;
	}
	if(rs == 0)
	{
		std::cerr << _("Server didn't answered for given timeout.")
		          << std::endl;
		return result;
	}
	if(!FD_ISSET(sock_, &rfds))
	{
		std::cerr << _("Select returned positive, but sock"
		               " is not in rfds.") << std::endl;
		return result;
	}

	const int recvbufsz = 2000;
	char * recvbuf = (char*)malloc(recvbufsz);
	memset(recvbuf, 0, recvbufsz);
	rs = recvfrom(sock_, recvbuf, recvbufsz - 1, 0,
	              (struct sockaddr*)&addr_, &addrln_);
	if(rs < 0)
	{
		std::cerr << _("Error receiving data.")  << " "
		          << _(" Message: ")<< strerror(GET_LAST_SOCK_ERROR());
		return result;
	}
	result.assign(recvbuf, recvbuf + rs);
	free(recvbuf);
	return result;
}


