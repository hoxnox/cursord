/**@author Merder Kim <hoxnox@gmail.com>
 * @date 20131225 15:48:29
 * @copyright Merder Kim
 *
 * Simple cursord client. It can send GET, STOP and SPEED commands to
 * the cursord server and print received answers.*/

#include <tclap/CmdLine.h>

#include <string>
#include <iostream>
#include <memory>

#include <cursordconf.h>
#include <string.hpp>
#include <gettext.h>
#include <nx_socket.h>
#include <simpleudp.hpp>

#include <logging.h>

typedef struct sockaddr_storage Sockaddr;

enum Command_t
{
	COMMAND_GET = 0,
	COMMAND_STOP  = 1,
	COMMAND_SPEED = 2
};

struct Config
{
	Config(int argc, char * argv[])
	{
		try
		{
			init(argc, argv);
		}
		catch(TCLAP::ArgException &e)
		{
			std::cerr << "Error: " << e.error()
			          << " " << e.argId() << std::endl;
		}
	}
	Command_t cmd;
	Sockaddr  addr;
private:
	void init(int argc, char * argv[]);
	Config() {} // disabled;
};

void
Config::init(int argc, char * argv[])
{
	std::stringstream version;
	version << CURSORD_VERSION_MAJOR << "." << CURSORD_VERSION_MINOR;
	TCLAP::CmdLine cmd_line(_("Information distributor."), ' ', version.str());

	TCLAP::ValueArg<std::string> arg_command(
			"c", "command", _("Command to send."),
			                 true, "speed", "string", cmd_line);
	TCLAP::ValueArg<std::string> arg_address(
			"H", "host", _("Cursor name/address."),
			              false, "127.0.0.1", "string", cmd_line);
	TCLAP::ValueArg<unsigned short> arg_port(
			"P", "port", _("Cursor port."), false, 9553,
			             "unsigned short", cmd_line);
	cmd_line.parse( argc, argv );

	std::string addr_str = arg_address.getValue();
	memset(&addr, 0, sizeof(Sockaddr));
	if( MakeSockaddr((sockaddr*)&addr, addr_str.c_str(), addr_str.length(),
			htons(arg_port.getValue())) < 0 )
	{
		throw TCLAP::ArgException(_("Invalid host, or port"), "H");
	}

	nx::String cmd_s = nx::String::fromUTF8(
			arg_command.getValue()).trim().toLower();
	if (cmd_s == "get")
		cmd = COMMAND_GET;
	else if (cmd_s == "stop")
		cmd = COMMAND_STOP;
	else if (cmd_s == "speed")
		cmd = COMMAND_SPEED;
	else
		throw TCLAP::ArgException(_("Invalid command."), "c");
}

int
main(int argc, char* argv[])
{
	Config cfg(argc, argv);
	SimpleUDP udp((struct sockaddr*)&(cfg.addr));
	switch(cfg.cmd)
	{
		case COMMAND_GET:
			if (udp.Send("GET") == 3)
				std::cout << udp.Recv() << std::endl;
			break;
		case COMMAND_SPEED:
			if (udp.Send("SPEED") == 5)
				std::cout << udp.Recv() << std::endl;
			break;
		case COMMAND_STOP:
			udp.Send("STOP");
			break;
	}
	return 0;
}

