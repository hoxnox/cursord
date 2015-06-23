/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

#include "cursor.hpp"

#include <sstream>
#include <ctime>
#include <iomanip>
#include <limits>
#include <unistd.h>

namespace cursor
{

/**@class Cursor
 * @brief Cursor interface
 *
 * Defines common interface to all Cursor classes and encapsulates all main
 * network operations.*/

Cursor::Cursor()
	: state_(0)
	, bufsz_(1000)
	, shared_(false)
	, shared_curr_(0)
	, shared_total_(0)
{
}

Cursor::Cursor(const size_t shared_curr, const size_t shared_total)
	: state_(0)
	, bufsz_(1000)
	, shared_(true)
	, shared_curr_(shared_curr)
	, shared_total_(shared_total)
{
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
}

/**@brief Start cursor server*/
void Cursor::Run(const char * url)
{
	int sock = nn_socket(AF_SP, NN_PAIR);
	if (sock < 0)
	{
		LOG(ERROR) << _("Error creating socket.")
		           << _(" Message: ") << nn_strerror(errno);
		return;
	}
	if (nn_bind(sock, url) < 0)
	{
		LOG(ERROR) << _("Error binding socket.")
		           << _(" Message: ") << nn_strerror(errno);
		return;
	}
	LOG(INFO) << _("Starting cursor.") << _(" URL: ") << url;

	while( true )
	{
		struct nn_pollfd pfd[1];
		pfd[0].fd = sock;
		pfd[0].events = NN_POLLIN;
		int rs = nn_poll (pfd, 1, 1000);
		if (rs == 0)
		{
			if(state_ & STATE_STOP)
			{
				nn_shutdown(sock, 0);
				break;
			}
			continue;
		}
		if (rs == -1)
		{
			if (rs != EINTR)
			{
				LOG(ERROR) << _("Error calling select.")
				           << _(" Message: ") << nn_strerror(errno);
				state_ = STATE_STOP | STATE_ERROR;
			}
			continue;
		}
		if (!(pfd[0].revents & NN_POLLIN))
		{
			LOG(ERROR) << _("Select returned positive, but sock is not in rfds.");
			continue;
		}
		char *buf = NULL;
		int bufsz = nn_recv (sock, &buf, NN_MSG, 0);
		if (bufsz < 0)
		{
			LOG(ERROR) << _("Error data receiving.")
			           << _(" Message: ") << nn_strerror(errno);
			state_ = STATE_STOP | STATE_ERROR;
			nn_freemsg (buf);
			continue;
		}

		nx::String request = nx::String::fromUTF8(
				std::string(buf, buf + bufsz)).trim().toLower();
		nn_freemsg (buf);
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
		rs = nn_send(sock, reply_utf8.c_str(), reply_utf8.length(), 0);
		if(rs < 0)
		{
			LOG(ERROR) << _("Error sending reply.")
			           << _(" Message: ") << nn_strerror(errno);
			state_ = STATE_STOP | STATE_ERROR;
		}
	}
	nn_shutdown(sock, 0);
	nn_close(sock);
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

