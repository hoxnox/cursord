/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include <tclap/CmdLine.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>

#include <string>
#include <iostream>
#include <memory>

#include <cursordconf.h>
#include <string.hpp>
#include <gettext.h>
#include <nx_socket.h>

#include <logging.h>

typedef struct sockaddr_storage Sockaddr;

void ReadCB(bufferevent *bev, void *ptr)
{
	const int bufsz = 2000;
	char *buf = (char*)malloc(bufsz);
	memset(buf, 0, bufsz);
	/*Получаем данные из буфера*/
	struct evbuffer *input = bufferevent_get_input(bev);
	int n;
	std::cout << std::endl << "===DATA BEGIN===" << std::endl;
	while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0)
		std::cout << buf;
	std::cout << std::endl << "====DATA END====" << std::endl;
}

void EventCB(bufferevent *bev, short events, void *ptr)
{
	if(EVUTIL_SOCKET_ERROR() == EINPROGRESS)
		return;
	if(events & BEV_EVENT_CONNECTED)
		return;
	if(!(events & BEV_EVENT_ERROR) && !(events & BEV_EVENT_TIMEOUT)
			&& !(events & BEV_EVENT_EOF))
	{
		std::cerr << _("Unexpected event.") << std::endl;
		return;
	}
	if((events & BEV_EVENT_TIMEOUT) || (events & BEV_EVENT_EOF))
	{
		std::cerr << _("Timeout") << std::endl;
		return;
	}
	std::cerr << _("ERROR: ") << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR())
		<< std::endl;
}

int main(int argc, char* argv[])
{
	try
	{
		std::stringstream CURSORD_VERSION_STR;
		CURSORD_VERSION_STR << CURSORD_VERSION_MAJOR << "." << CURSORD_VERSION_MINOR;
		TCLAP::CmdLine cmd(_("no description yet"), ' ', CURSORD_VERSION_STR.str());

		TCLAP::ValueArg<std::string> arg_command(
				"c", "command", _("Command to send."),
				                 true, "generator", "string", cmd);
		TCLAP::ValueArg<std::string> arg_address(
				"H", "host", _("Cursor name/address."),
				              false, "127.0.0.1", "string", cmd);
		TCLAP::ValueArg<unsigned short> arg_port(
				"P", "port", _("Cursor port."), false, 9553, "unsigned short", cmd);
		cmd.parse( argc, argv );


		std::string addr_str = arg_address.getValue();
		Sockaddr addr;
		memset(&addr, 0, sizeof(Sockaddr));
		if( MakeSockaddr((sockaddr*)&addr, addr_str.c_str(), addr_str.length(), 
				htons(arg_port.getValue())) < 0 )
		{
			throw TCLAP::ArgException(_("Invalid host, or port"), "H");
		}


		event_base* evbase = event_base_new();
		SOCKET sock = socket(addr.ss_family, SOCK_DGRAM, 0);
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
		bufferevent * bev = bufferevent_socket_new(evbase, sock, BEV_OPT_CLOSE_ON_FREE);
		struct timeval tv;
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		if(bufferevent_set_timeouts(bev, &tv, &tv) < 0)
		{
			std::cerr << _("Error setting timeouts to bufferevent") << std::endl;
			return 0;
		}
		bufferevent_setcb(bev, ReadCB, NULL, EventCB, evbase);
		if(bufferevent_enable(bev, EV_READ|EV_WRITE) < 0)
		{
			std::cerr << _("Error enabling read and write events on bufferevent") 
				<< std::endl;
			return 0;
		}

		nx::String command = nx::String::fromUTF8(arg_command.getValue()).trim().toLower();
		std::map<nx::String, std::string> commands;
		commands[L"get"] = "GET";
		commands[L"speed"] = "SPEED";
		commands[L"stop"] = "STOP";
		auto rs = commands.find(command);
		if( rs == commands.end() )
			throw TCLAP::ArgException(_("Invalid command."), "c");
		else
			if(bufferevent_write(bev, rs->second.c_str(), rs->second.length()) < 0)
				std::cerr << _("Error writing to bufferevent.") << std::endl;

		bufferevent_socket_connect(bev, (sockaddr*)&addr, sizeof(Sockaddr));
		event_base_dispatch(evbase);
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " " << e.argId() << std::endl;
	}
	return 0;
}
