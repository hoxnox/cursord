/**@author $username$ <$usermail$>
 * @date $date$
 * (C) $username$ */

#include <tclap/CmdLine.h>
#include <event.h>
#include <cursordconf.h>

#include <sstream>


int main(int argc, char * argv[])
{
	try
	{
		std::stringstream CURSORD_VERSION_STR;
		CURSORD_VERSION_STR << CURSORD_VERSION_MAJOR << "." << CURSORD_VERSION_MINOR;
		TCLAP::CmdLine cmd("Command description message", ' ', CURSORD_VERSION_STR.str());

		TCLAP::ValueArg<std::string> arg_type(
				"t", "type", "Cursor type", true, "generator", "string", cmd);
		TCLAP::ValueArg<std::string> arg_address(
				"a", "address", "Server address", false, "127.0.0.1", "string", cmd);
		TCLAP::ValueArg<unsigned short> arg_port(
				"p", "port", "Server port", false, 9553, "unsigned short", cmd);

		cmd.parse( argc, argv );
		std::cout << arg_address.getValue() << ":" << arg_port.getValue() << std::endl;
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
	}
	return 0;
}
