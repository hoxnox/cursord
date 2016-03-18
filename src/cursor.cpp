/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

#include "cursor.hpp"

#include <sstream>
#include <ctime>
#include <iomanip>
#include <limits>
#include <unistd.h>
#include <fstream>
#include <Utils.hpp>

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
	, shared_curr_(1)
	, shared_total_(1)
{
}

Cursor::~Cursor()
{
	if(buf_.empty())
		LOG(INFO) << _("Cursor destruction: Buffer is empty.");
	else
		LOG(INFO) << _("Cursor destruction: ") << buf_.front();
}

typedef std::vector<std::string> Lines;

/**@brief Start cursor server*/
void Cursor::Run(const Config&& cfg)
{
	if (cfg.urls.empty())
	{
		LOG(INFO) << "No URL to bind on. Quit.";
		return;
	}
	shared_curr_ = cfg.shared_curr;
	shared_total_ = cfg.shared_total;

	std::vector<MixerT::InRangeT> fiters;
	std::vector<std::shared_ptr<std::ifstream>> ifiles;
	for (auto fname : cfg.extra_fnames)
	{
		ifiles.emplace_back(new std::ifstream(fname.c_str()));
		if (!ifiles.back()->is_open())
		{
			LOG(ERROR) << _("Error file opening.")
			           << _(" Filename: \"") << fname << "\"";
			continue;
		}
		fiters.emplace_back(MixerT::InRangeT(LineIterT(*ifiles.back()), LineIterT()));
	}
	extra_delim_ = cfg.extra_delim;
	extra_state_.reset(new MixerT(fiters,
		cfg.extra_mix ? MixerT::MIX_SHUFFLE : MixerT::MIX_NONE, 0xABCDEF0F));

	std::vector<int> socks;
	for (auto url : cfg.urls)
	{
		int sock = nn_socket(AF_SP, NN_PAIR);
		if (sock < 0)
		{
			LOG(ERROR) << _("Error creating socket.")
			           << _(" Message: ") << nn_strerror(errno);
			return;
		}
		if (nn_bind(sock, url.c_str()) < 0)
		{
			LOG(ERROR) << _("Error binding socket.")
			           << _(" Message: ") << nn_strerror(errno);
			return;
		}
		LOG(INFO) << _("Starting cursor.") << _(" URL: ") << url;
		socks.push_back(sock);
	}

	struct nn_pollfd* pfd = new nn_pollfd[socks.size()];
	while( true )
	{
		for (size_t i = 0; i < socks.size(); ++i)
		{
			pfd[i].fd = socks[i];
			pfd[i].events = NN_POLLIN;
		}
		int rs = nn_poll (pfd, socks.size(), 1000);
		if (rs == 0)
		{
			if(state_ & STATE_STOP)
				break;
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
		for (size_t i = 0; i < socks.size(); ++i)
		{
			if (pfd[i].revents & NN_POLLIN)
			{
				char *buf = NULL;
				int bufsz = nn_recv (pfd[i].fd, &buf, NN_MSG, 0);
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
				rs = nn_send(pfd[i].fd, reply_utf8.c_str(), reply_utf8.length(), 0);
			}
		}
		if(rs < 0)
		{
			LOG(ERROR) << _("Error sending reply.")
			           << _(" Message: ") << nn_strerror(errno);
			state_ = STATE_STOP | STATE_ERROR;
		}
	}
	delete [] pfd;
	for (auto sock : socks)
	{
		nn_shutdown(sock, 0);
		nn_close(sock);
	}
}

int Cursor::Next(const size_t count, std::deque<nx::String>& buf)
{
	int rs = 0;
	if (shared_total_ != 0 && shared_curr_ != 0 && shared_curr_ < shared_total_)
	{
		if (extra_state_ && extra_state_->IsReady())
		{  // +extra with mixing
			std::string rs = extra_state_->Get(extra_delim_);
			buf.emplace_back(rs.begin(), rs.end());
			++(*extra_state_);
			rs = 1;
		}
		else
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
	}
	else
	{
		++(*extra_state_);
		rs = do_next(count, buf);
	}
	return rs;
}

} //namespace

