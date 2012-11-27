/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor.hpp"
#include <nx_socket.h>

namespace cursor
{

/**@class Cursor
 * @brief Cursor interface
 *
 * Defines common interface to all Cursor classes and encapsulates all main
 * network operations.*/

/**@brief constructor
 * @param addr Address to bind server.*/
Cursor::Cursor(Sockaddr addr)
	: laddr_(addr)
	 ,evbase_(event_base_new())
	 ,bev_(NULL)
	 ,state_(0)
	 ,recvbuf_(NULL)
	 ,recvbufsz_(2000)
{
	timeout_.tv_sec = 30;
	timeout_.tv_usec = 0;
	recvbuf_ = (char *)malloc(recvbufsz_);
	memset(recvbuf_, 0, recvbufsz_);
}

Cursor::~Cursor()
{
	if(bev_)
		bufferevent_free(bev_);
}

void Cursor::readCallback(bufferevent *bev, void *ptr)
{
	Cursor* curs = (Cursor*)ptr;
	nx::String reply = L"Hello!";
	if(!curs)
	{
		LOG(ERROR) << _("Error getting parent-class on read callback");
		return;
	}
	if(curs->state_ & STATE_STOP)
	{
		reply = L"END";
		LOG(INFO) << _("Received GET in STATE_STOP resetting timeout.");
		if(bufferevent_set_timeouts(bev, &curs->timeout_, &curs->timeout_) < 0)
		{
			LOG(ERROR) << _("Error setting timeouts to bufferevent.");
			return;
		}
	}
	memset(curs->recvbuf_, 0, curs->recvbufsz_);
	Sockaddr raddr;
	socklen_t raddrln = sizeof(Sockaddr);
	SOCKET sock = bufferevent_getfd(bev);
	int rs = recvfrom(sock, curs->recvbuf_, curs->recvbufsz_, 0,
			(sockaddr*)&raddr, &raddrln);
	if(rs <= 0)
	{
		curs->state_ = STATE_ERROR | STATE_STOP;
		LOG(ERROR) << _("Error receiving data.") << " "
			<< _("Message: ") << strerror(GET_LAST_SOCK_ERROR());
		return;
	}
	rs = sendto(sock, reply.data(), reply.length(), 0, (sockaddr*)&raddr, raddrln);
	if(rs <= 0)
	{
		curs->state_ = STATE_ERROR | STATE_STOP;
		LOG(ERROR) << _("Error sending data");
		return;
	}
}

void Cursor::eventCallback(bufferevent *bev, short events, void *ptr)
{
	Cursor* curs = (Cursor*)ptr;
	if(!curs)
	{
		LOG(ERROR) << _("Error getting parent-class on event callback");
		return;
	}
	if(EVUTIL_SOCKET_ERROR() == EINPROGRESS)
		return;
	if(events & BEV_EVENT_CONNECTED)
		return;
	if(events & BEV_EVENT_TIMEOUT)
	{
		if(curs->state_ & STATE_STOP)
		{
			LOG(INFO) << _("Timeout on STATE_STOP.");
			return;
		}
		if(bufferevent_set_timeouts(bev, &curs->timeout_, &curs->timeout_) < 0)
			LOG(ERROR) << _("Error setting timeouts to bufferevent.");
		if(bufferevent_enable(bev, EV_READ|EV_WRITE) < 0)
			LOG(ERROR) << _("Error enabling read and write event on bufferevent.");
		return;
	}
	if(!(events & BEV_EVENT_ERROR) && !(events & BEV_EVENT_EOF))
	{
		LOG(ERROR) << _("Unexpected event.");
		return;
	}
	LOG(ERROR) << "ERROR: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
}

int Cursor::initBufferevent()
{
	SOCKET sock = socket(laddr_.ss_family, SOCK_DGRAM, 0);
	if(!IS_VALID_SOCK(sock))
	{
		LOG(ERROR) << _("Error socket initialization.") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		return -1;
	}
	if(SetNonBlock(sock) < 0)
	{
		LOG(ERROR) << _("Error switching socket to non-block mode.") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return -1;
	}
	if(SetReusable(sock) < 0)
	{
		LOG(ERROR) << _("Error making socket reusable") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return -1;
	}
	if( bind(sock, (sockaddr*)&laddr_, sizeof(Cursor::Sockaddr)) < 0 )
	{
		LOG(ERROR) << _("Error binding docket.") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return -1;
	}
	bufferevent * bev_ = bufferevent_socket_new(evbase_, sock, BEV_OPT_CLOSE_ON_FREE);
	if(!bev_)
	{
		LOG(ERROR) << _("Error creating new bufferevent.");
		return -1;
	}
	if(bufferevent_set_timeouts(bev_, &timeout_, &timeout_) < 0)
	{
		LOG(ERROR) << _("Error setting timeouts to bufferevent.");
		return -1;
	}
	bufferevent_setcb(bev_, Cursor::readCallback, 
	                        NULL,
	                        Cursor::eventCallback, this);
	if(bufferevent_enable(bev_, EV_READ|EV_WRITE) < 0)
	{
		LOG(ERROR) << _("Error enabling read and write event on bufferevent.");
		return -1;
	}
	return 0;
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
	if( initBufferevent() < 0 )
		return;
	LOG(INFO) << _("Starting cursor.") << " " << _("Address") << ": " << tmp << " "
		<< _("port") << ": " << ntohs(GetPort((sockaddr*)&laddr_));
	event_base_dispatch(evbase_);
}

} //namespace

