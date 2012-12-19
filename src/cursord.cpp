/**@author $username$ <$usermail$>
 * @date $date$
 * (C) $username$ */

#include <tclap/CmdLine.h>

#include <sstream>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>

#include <cursordconf.h>
#include <nx_socket.h>
#include <string.hpp>
#include <gettext.h>
#include <cursor.hpp>
#include <cursor_generator.hpp>
#include <cursor_file.hpp>

#ifndef CFG_WITHOUT_ODBC
#include <cursor_odbc.hpp>
#endif // CFG_WITHOUT_ODBC


using namespace nx;
using namespace cursor;

typedef std::map<String, String> Args;
typedef struct sockaddr_storage Sockaddr;

Args parse_args(const String args)
{
	Args result;
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
					= arg.substr(peq + 1, arg.length());
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
				= arg.substr(peq + 1, arg.length());
		}
	}
	return result;
}

void RunCursor(const std::string type, const Args& args, const Sockaddr& addr)
{
	Cursor * curs;
	if(type == "generator")
		curs = new CursorGenerator(addr, args);
	else if(type == "file")
		curs = new CursorFile(addr, args);
#ifndef CFG_WITHOUT_ODBC
	else if(type == "odbc")
		curs = new CursorODBC(addr, args);
#endif // CFG_WITHOUT_ODBC
	else
		throw TCLAP::ArgException(_("Unknown cursor type"), "t");
	curs->Run();
	delete curs;
	return;
}

int main(int argc, char * argv[])
{
	try
	{
		std::stringstream CURSORD_VERSION_STR;
		CURSORD_VERSION_STR << CURSORD_VERSION_MAJOR << "." << CURSORD_VERSION_MINOR;
		TCLAP::CmdLine cmd(_("no description yet"), ' ', CURSORD_VERSION_STR.str());

		TCLAP::ValueArg<std::string> arg_type(
				"t", "type", _("Cursor type. Valid types are: generator, file, odbc"),
				                 true, "generator", "string", cmd);
		TCLAP::ValueArg<std::string> arg_arg(
				"a", "argument", _("Cursor arguments (depend on type)."
					"Each argument has the following format: <name>=<value>."
					"Arguments splitted by ';'."), 
					false, "", "string", cmd);
		TCLAP::ValueArg<std::string> arg_address(
				"H", "host", _("Server address"), false, "127.0.0.1", "string", cmd);
		TCLAP::ValueArg<unsigned short> arg_port(
				"P", "port", _("Server port"), false, 9553, "unsigned short", cmd);

		cmd.parse( argc, argv );
		Args args = parse_args(
				String::fromUTF8(arg_arg.getValue()));
		std::string addr_str = arg_address.getValue();
		Sockaddr addr;
		memset(&addr, 0, sizeof(Sockaddr));
		if( MakeSockaddr((struct sockaddr*)&addr, addr_str.c_str(), addr_str.length(), 
				htons(arg_port.getValue())) < 0 )
		{
			throw TCLAP::ArgException(_("Invalid host, or port"), "H");
		}
		RunCursor(arg_type.getValue(), args, addr);
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}

