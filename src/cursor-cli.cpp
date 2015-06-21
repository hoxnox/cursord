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

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

#include <cursordconf.h>
#include <string.hpp>
#include <gettext.h>

#include <logging.h>

typedef struct sockaddr_storage Sockaddr;

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
	std::string cmd;
	std::string url;
private:
	void init(int argc, char * argv[]);
	Config() {} // disabled;
};

void
Config::init(int argc, char * argv[])
{
	std::stringstream version;
	version << CURSORD_VERSION_MAJOR
	        << "." << CURSORD_VERSION_MINOR
	        << "." << CURSORD_VERSION_PATCH;
	TCLAP::CmdLine cmd_line(_("Cursord ver."), ' ', version.str());

	TCLAP::ValueArg<std::string> arg_command(
			"c", "command", _("Command to send."),
			                 true, "speed", "string", cmd_line);
	TCLAP::ValueArg<std::string> arg_url(
			"u", "url", _("Server url."),
			              false, "ipc:///tmp/cursord.ipc", "string", cmd_line);
	cmd_line.parse(argc, argv);
	url = arg_url.getValue();
	cmd = nx::String::fromUTF8(arg_command.getValue()).trim().toUpper().toASCII();
	if (cmd != "GET" && cmd != "STOP" && cmd != "SPEED")
		throw TCLAP::ArgException(_("Invalid command."), "c");
}

int
main(int argc, char* argv[])
{
	Config cfg(argc, argv);
	int sock = nn_socket (AF_SP, NN_PAIR);
	if (sock < 0)
	{
		std::cerr << _("Error creating socket.")
		          << _(" Message: ") << nn_strerror(errno);
		return 1;
	}
	if (nn_connect(sock, cfg.url.c_str()) < 0)
	{
		std::cerr << _("Error connecting.")
		          << _(" URL: ") << cfg.url
		          << _(" Message: ") << nn_strerror(errno);
		return 1;
	}
	if (nn_send(sock, cfg.cmd.c_str(), cfg.cmd.length(), 0) < 0)
	{
		std::cerr << _("Error sending data.")
		          << _(" Message: ") << nn_strerror(errno);
		return 1;
	}
	if (cfg.cmd != "STOP")
	{
		char* buf;
		int bytes = nn_recv (sock, &buf, NN_MSG, 0);
		if (bytes < 0)
		{
			std::cerr << _("Error receiving data.")
			          << _(" URL: ") << cfg.url
			          << _(" Message: ") << nn_strerror(errno);
					  return 1;
		}
		std::cout << std::string(buf, buf + bytes) << std::endl;
		nn_freemsg (buf);
	}
	nn_shutdown(sock, 0);
	return 0;
}

