/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include <tclap/CmdLine.h>
#include <cursordconf.h>
#include <string.hpp>

typedef struct sockaddr_storage Sockaddr;

int main(intargc, char* argv[])
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

		nx::String command = nx::String::fromUTF8(arg_command.getValue()).trim().tolower();
		else if(command == "get")
		{
		}
		else if(command == "speed")
		{
		}
		else if(command == "stop")
		{
		}
		else
		{
			throw TCLAP::ArgException(_("Invalid command."), "c");
		}
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
	}
	return 0;
}
