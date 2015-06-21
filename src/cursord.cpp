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

inline int parse_shared(const std::string shared, size_t& shared_current,
		size_t& shared_total)
{
	size_t pos = shared.find('/');
	if(pos == 0 || pos >= shared.length() - 1 || pos == std::string::npos)
	{
		shared_current = 0;
		shared_total = 0;
	}
	shared_current = (size_t)atol(shared.substr(0, pos).c_str());
	shared_total = (size_t)atol(shared.substr(pos + 1, shared.length() - pos - 1).c_str());
	return 0;
}

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

void RunCursor(const std::string type,
               const Args& args,
               const char* url,
               const size_t shared_curr,
               const size_t shared_total)
{
	bool shared = true;
	if((shared_total == 0 || shared_curr == 0) || shared_curr > shared_total)
		shared = false;
	Cursor * curs;
	if(type == "generator")
	{
		if(shared)
			curs = new CursorGenerator(args, shared_curr, shared_total);
		else
			curs = new CursorGenerator(args);
	}

	else if(type == "file")
	{
		if(shared)
			curs = new CursorFile(args, shared_curr, shared_total);
		else
			curs = new CursorFile(args);
	}

#ifndef CFG_WITHOUT_ODBC
	else if(type == "odbc")
	{
		if(shared)
			curs = new CursorODBC(args, shared_curr, shared_total);
		else
			curs = new CursorODBC(args);
	}
#endif // CFG_WITHOUT_ODBC
	else
		throw TCLAP::ArgException(_("Unknown cursor type"), "t");
	curs->Run(url);
	delete curs;
	return;
}

int main(int argc, char * argv[])
{
	try
	{
		std::stringstream version;
		version << CURSORD_VERSION_MAJOR
		        << "." << CURSORD_VERSION_MINOR
		        << "." << CURSORD_VERSION_PATCH;
		TCLAP::CmdLine cmd(_("Cursord ver."), ' ', version.str());

		TCLAP::ValueArg<std::string> arg_type(
			"t", "type", _("Cursor type. Valid types are: generator, file, odbc"),
			                 true, "generator", "string", cmd);
		TCLAP::ValueArg<std::string> arg_shared(
			"s", "shared", _("Cursor is shared. You must specify total"
				"number and this number. E.g. 2/4 means"
				"this number is 2 and total is 4."),
				false, "", "string", cmd);
		TCLAP::ValueArg<std::string> arg_arg(
			"a", "argument", _("Cursor arguments (depend on type)."
				"Each argument has the following format: <name>=<value>."
				"Arguments splitted by ';'."), 
				false, "", "string", cmd);
		TCLAP::ValueArg<std::string> arg_url(
				"u", "url", _("Server url"),
				false, "ipc:///tmp/cursord.ipc", "string", cmd);

		cmd.parse(argc, argv);
		Args args = parse_args(String::fromUTF8(arg_arg.getValue()));
		std::string url = arg_url.getValue();
		size_t shared_curr = 0;
		size_t shared_total = 0;
		parse_shared(arg_shared.getValue(), shared_curr, shared_total);
		RunCursor(arg_type.getValue(), args, url.c_str(), shared_curr, shared_total);
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}

