/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include <tclap/CmdLine.h>

#include <string>
#include <iostream>
#include <memory>

#include <cursordconf.h>
#include <string.hpp>
#include <gettext.h>
#include <nx_socket.h>

#include <logging.h>

typedef struct sockaddr_storage Sockaddr;

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
		socklen_t addrln = sizeof(Sockaddr);
		memset(&addr, 0, addrln);
		if( MakeSockaddr((sockaddr*)&addr, addr_str.c_str(), addr_str.length(), 
				htons(arg_port.getValue())) < 0 )
		{
			throw TCLAP::ArgException(_("Invalid host, or port"), "H");
		}


		nx::String command = nx::String::fromUTF8(arg_command.getValue()).trim().toLower();
		std::map<nx::String, std::string> commands;
		commands[L"get"] = "GET";
		commands[L"speed"] = "SPEED";
		commands[L"stop"] = "STOP";
		auto com = commands.find(command);
		if( com == commands.end() )
			throw TCLAP::ArgException(_("Invalid command."), "c");


		SOCKET sock = socket(addr.ss_family, SOCK_DGRAM, 0);
		if(!IS_VALID_SOCK(sock))
		{
			std::cerr << _("Error socket initialization.") << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR())
				<< std::endl;
			return 0;
		}
		if(SetNonBlock(sock) < 0)
		{
			std::cerr << _("Error switching socket to non-block mode.") << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR())
				<< std::endl;
			close(sock);
			return 0;
		}
		if(SetReusable(sock) < 0)
		{
			std::cerr << _("Error making socket reusable") << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR())
				<< std::endl;
			close(sock);
			return 0;
		}


		int rs = sendto(sock, com->second.data(), com->second.length(), 0,
				(sockaddr*)&addr, addrln);
		if( rs <= 0 )
		{
			std::cerr << _("Error sending command.") << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR())
				<< std::endl;
			return 0;
		}
		struct timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		rs = select(sock + 1, &rfds, NULL, NULL, &timeout);
		if(rs < 0)
		{
			std::cerr << _("Error receiving data.") << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR())
				<< std::endl;
			return 0;
		}
		if(rs == 0)
		{
			std::cerr << _("Server didn't answered for given timeout.") << std::endl;
			return 0;
		}
		if(!FD_ISSET(sock, &rfds))
		{
			std::cerr << _("Select returned positive, but sock is not in rfds.")
				<< std::endl;
			return 0;
		}


		const int recvbufsz = 2000;
		char * recvbuf = (char*)malloc(recvbufsz);
		memset(recvbuf, 0, recvbufsz);
		rs = recvfrom(sock, recvbuf, recvbufsz - 1, 0, (sockaddr*)&addr, &addrln);
		if(rs <= 0)
		{
			std::cerr << _("Error receiving data.")  << " "
				<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
			return 0;
		}
		std::cout << recvbuf << std::endl;
		free(recvbuf);
		return 0;
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " " << e.argId() << std::endl;
	}
	return 0;
}

