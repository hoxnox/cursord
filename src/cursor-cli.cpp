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
		memset(&addr, 0, sizeof(Sockaddr));
		if( MakeSockaddr((sockaddr*)&addr, addr_str.c_str(), addr_str.length(), 
				htons(arg_port.getValue())) < 0 )
		{
			throw TCLAP::ArgException(_("Invalid host, or port"), "H");
		}


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

		nx::String command = nx::String::fromUTF8(arg_command.getValue()).trim().toLower();
		std::map<nx::String, std::string> commands;
		commands[L"get"] = "GET";
		commands[L"speed"] = "SPEED";
		commands[L"stop"] = "STOP";
		auto rs = commands.find(command);
		if( rs == commands.end() )
			throw TCLAP::ArgException(_("Invalid command."), "c");
		else
			;
			// TODO:
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " " << e.argId() << std::endl;
	}
	return 0;
}
