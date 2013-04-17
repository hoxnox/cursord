/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor.hpp"
#include <nx_socket.h>

#include <sstream>
#include <ctime>
#include <iomanip>
#include <limits>

namespace cursor
{

/**@class Cursor
 * @brief Cursor interface
 *
 * Defines common interface to all Cursor classes and encapsulates all main
 * network operations.*/

/**@brief constructor
 * @param addr Address to bind server.*/
Cursor::Cursor(const Sockaddr addr)
	: state_(0)
	, recvbuf_(NULL)
	, recvbufsz_(2000)
	, bufsz_(1000)
	, shared_(false)
	, shared_curr_(0)
	, shared_total_(0)
{
	init(addr);
}

void Cursor::init(const Sockaddr addr)
{
	timeout_.tv_sec = 30;
	timeout_.tv_usec = 0;
	recvbuf_ = (char *)malloc(recvbufsz_);
	memset(recvbuf_, 0, recvbufsz_);
	memset(&laddr_, 0, sizeof(Sockaddr));
	memcpy(&laddr_, &addr, sizeof(Sockaddr));
}

Cursor::Cursor(const Sockaddr addr, const size_t shared_curr, const size_t shared_total)
	: state_(0)
	, recvbuf_(NULL)
	, recvbufsz_(2000)
	, bufsz_(1000)
	, shared_(true)
	, shared_curr_(shared_curr)
	, shared_total_(shared_total)
{
	init(addr);
	if(shared_total_ < shared_curr_ || shared_curr_ == 0 || shared_total_ == 0)
	{
		shared_ = false;
		shared_curr_ = 0;
		shared_total_ = 0;
	}
}


Cursor::~Cursor()
{
	if(buf_.empty())
		LOG(INFO) << _("Cursor destruction: Buffer is empty.");
	else
		LOG(INFO) << _("Cursor destruction: ") << buf_.front();

	if(recvbuf_)
		free(recvbuf_);
}

inline SOCKET init_socket(Cursor::Sockaddr& laddr)
{
	SOCKET sock = socket(laddr.ss_family, SOCK_DGRAM, 0);
	if(!IS_VALID_SOCK(sock))
	{
		LOG(ERROR) << _("Error socket initialization.") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		return INVALID_SOCKET;
	}
	if(SetNonBlock(sock) < 0)
	{
		LOG(ERROR) << _("Error switching socket to non-block mode.") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return INVALID_SOCKET;
	}
	if(SetReusable(sock) < 0)
	{
		LOG(ERROR) << _("Error making socket reusable") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return INVALID_SOCKET;
	}
	if( bind(sock, (sockaddr*)&laddr, sizeof(Cursor::Sockaddr)) < 0 )
	{
		LOG(ERROR) << _("Error binding docket.") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return INVALID_SOCKET;
	}
	return sock;
}

/**@brief Start cursor server*/
void Cursor::Run()
{
	const int tmpsz = 50;
	char * tmp = (char *)malloc(tmpsz);
	memset(tmp, 0, tmpsz);
	if(inet_ntop(laddr_.ss_family, GetAddr((sockaddr*)&laddr_), tmp, tmpsz) == NULL)
	{
		LOG(ERROR) << _("Wrong address.") << " "
			<< _("Message") << ": " << strerror(errno);
		return;
	}
	SOCKET sock = init_socket(laddr_);
	if( !IS_VALID_SOCK(sock) )
		return;

	LOG(INFO) << _("Starting cursor.") << " " << _("Address") << ": " << tmp << " "
		<< _("port") << ": " << ntohs(GetPort((sockaddr*)&laddr_));

	while( true )
	{
		int rs = 0;
		fd_set rfds;

		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		struct timeval tm = timeout_;

		rs = select(sock + 1, &rfds, NULL, NULL, &tm);
		if(rs == 0)
		{
			if(state_ & STATE_STOP)
				break;
			continue;
		}
		if(rs < 0)
		{
			if( ( rs = GET_LAST_SOCK_ERROR() ) != EINTR ) // не просто прерывание
			{
				LOG(ERROR) << _("Error calling select") << " "
					<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
				state_ = STATE_STOP | STATE_ERROR;
			}
			continue;
		}
		if(!FD_ISSET(sock, &rfds))
		{
			LOG(ERROR) << _("Select returned positive, but sock is not in rfds.");
			state_ = STATE_STOP | STATE_ERROR;
			continue;
		}
		Sockaddr raddr;
		socklen_t raddrln = sizeof(raddr);
		memset(recvbuf_, 0, recvbufsz_);
		int rs_ = recvfrom(sock, recvbuf_, recvbufsz_ - 1, 0, (sockaddr*)&raddr, &raddrln);
		if(rs_ < 0)
		{
			LOG(ERROR) << _("Error receiving data.") << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
			state_ = STATE_STOP | STATE_ERROR;
		}


		nx::String request = nx::String::fromUTF8(recvbuf_).trim().toLower();
		nx::String reply;
		if(request == "speed")
		{
			reply = nx::String::fromASCII(speedometer_.AVGSpeedS()) + L" "
			      + nx::String::fromASCII(speedometer_.LastSpeedS());
		}
		else if(request == "stop")
		{
			LOG(INFO) << _("Received stop signal. Setting STATE_STOP.");
			state_ = state_ | STATE_STOP;
			continue;
		}
		else if(request == "get")
		{
			++speedometer_;
			if(state_ == STATE_STOP)
			{
				LOG(INFO) << _("Received GET in STATE_STOP");
				reply = L"END";
			}
			else
			{
				if(buf_.empty())
				{
					Next(bufsz_, buf_);
					if(buf_.empty())
					{
						LOG(INFO) << _("Cursor is empty. Stopping.");
						state_ = state_ | STATE_STOP;
						reply = L"END";
					}
					else
					{
						LOG(INFO) << "Renew buffer. Last element: " << buf_.back().toUTF8();
					}
				}
				if(!(state_ & STATE_STOP))
				{
					reply = buf_.front();
					buf_.pop_front();
				}
			}
		}
		else
		{
			LOG(WARNING) << _("Unknown request: ") << request.toUTF8();
			continue;
		}
		std::string reply_utf8 = reply.toUTF8();
		rs_ = sendto(sock, reply_utf8.data(), reply.length(), 0,
				(sockaddr*)&raddr, raddrln);
		if(rs_ < 0)
		{
			LOG(ERROR) << _("Error sending data.") << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
			state_ = STATE_STOP | STATE_ERROR;
		}
	}
}

int Cursor::Next(const size_t count, std::deque<nx::String>& buf)
{
	int rs;
	if(shared_)
	{
		std::deque<nx::String> tmpbuf;
		rs = do_next(count * shared_total_, tmpbuf);
		if(rs >= 0)
			for(size_t i = 0, j = shared_curr_ - 1; i < count && j < tmpbuf.size(); 
					++i, j = shared_curr_ - 1 + shared_total_*i )
			{
				buf.push_back(tmpbuf[j]);
			}
	}
	else
	{
		rs = do_next(count, buf);
	}
	return rs;
}

} //namespace

