/**@author $username$ <$usermail$>
 * @date $date$
 * (C) $username$ */

#include <tclap/CmdLine.h>
#include <event.h>
#include <cursordconf.h>

#include <sstream>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <string.hpp>
#include <nx_socket.h>

using namespace nx;

std::map<String, String> parse_args(const String args)
{
	std::map<String, String> result;
	size_t pbegin = 0, pcurr = 0;
	while( (pcurr = args.find_first_of(';', pbegin)) != String::npos)
	{
		if(pcurr - pbegin > 1)
		{
			String arg = args.substr(pbegin, pcurr - pbegin);
			size_t peq = arg.find_first_of('=');
			if( peq != String::npos && peq > 1 )
			{
				result[arg.substr(0, peq).trim().toLower()] 
					= arg.substr(peq + 1, arg.length()).trim().toLower();
			}
		}
		pbegin = pcurr + 1;
	}
	if(args.length() - pbegin > 1)
	{
		String arg = args.substr(pbegin, args.length() - pbegin);
		size_t peq = arg.find_first_of('=');
		if( peq != String::npos && peq > 1 )
		{
			result[arg.substr(0, peq).trim().toLower()] 
				= arg.substr(peq + 1, arg.length()).trim().toLower();
		}
	}
	return result;
}

void RunCursor(const std::string               type,
               const std::map<String, String>& args, 
               const sockaddr_storage*         addr)
{
	return;
}

int main(int argc, char * argv[])
{
	try
	{
		std::stringstream CURSORD_VERSION_STR;
		CURSORD_VERSION_STR << CURSORD_VERSION_MAJOR << "." << CURSORD_VERSION_MINOR;
		TCLAP::CmdLine cmd("Command description message", ' ', CURSORD_VERSION_STR.str());

		TCLAP::ValueArg<std::string> arg_type(
				"t", "type", "Cursor type. Valid types are: generator, file, odbc",
				                 true, "generator", "string", cmd);
		TCLAP::ValueArg<std::string> arg_arg(
				"a", "argument", "Cursor argument (depend on type)", 
				                 false, "", "string", cmd);
		TCLAP::ValueArg<std::string> arg_address(
				"H", "host", "Server address", false, "127.0.0.1", "string", cmd);
		TCLAP::ValueArg<unsigned short> arg_port(
				"P", "port", "Server port", false, 9553, "unsigned short", cmd);

		cmd.parse( argc, argv );
		std::cout << arg_address.getValue() << ":" << arg_port.getValue() << std::endl;
		std::map<String, String> args = parse_args(
				String::fromUTF8(arg_arg.getValue()));
		std::string addr_str = arg_address.getValue();
		sockaddr_storage addr;
		MakeSockaddr((sockaddr*)&addr, addr_str.c_str(), addr_str.length(), htons(arg_port.getValue()));
		RunCursor(arg_type.getValue(), args, &addr);
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
