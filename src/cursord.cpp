/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * (C) hoxnox */

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
		shared_current = 1;
		shared_total = 1;
		return -1;
	}
	shared_current = (size_t)atol(shared.substr(0, pos).c_str());
	shared_total = (size_t)atol(shared.substr(pos + 1, shared.length() - pos - 1).c_str());
	if (shared_current == 0 || shared_total == 0 || shared_current < shared_total)
	{
		shared_current = 1;
		shared_total = 1;
		return -1;
	}
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
               Cursor::Config&& cfg)
{
	Cursor * curs;
	if(type == "generator")
		curs = new CursorGenerator(args);
	else if(type == "file")
		curs = new CursorFile(args);
#ifndef CFG_WITHOUT_ODBC
	else if(type == "odbc")
			curs = new CursorODBC(args);
#endif // CFG_WITHOUT_ODBC
	else
		throw TCLAP::ArgException(_("Unknown cursor type"), "t");
	curs->Run(std::move(cfg));
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
		TCLAP::MultiArg<std::string> arg_url(
			"u", "url", _("Server url"),
			false, "string", cmd);
		TCLAP::MultiArg<std::string> arg_extra(
			"e", "extra", _("Extra files to append content."),
			false, "string", cmd);
		TCLAP::ValueArg<std::string> arg_extra_delim(
			"E", "extra-delim", _("Delimiter, used to separate extra files values. Defalut is ';'"),
				false, ";", "string", cmd);
		TCLAP::ValueArg<bool> arg_extra_mix(
			"M", "extra-mix", _("Mix extra files content. Defalt is false."),
			false, false, "bool", cmd);

		cmd.parse(argc, argv);
		Args args = parse_args(String::fromUTF8(arg_arg.getValue()));
		size_t shared_curr = 0;
		size_t shared_total = 0;
		if (parse_shared(arg_shared.getValue(), shared_curr, shared_total) == -1)
		{
			std::cerr << "Error parsing option shared."
			          << "Option value: " << arg_shared.getValue()
			          << std::endl;
		}
		RunCursor(arg_type.getValue(), args,
			Cursor::Config(std::move(arg_url.getValue()),
			               std::move(arg_extra.getValue()),
			               shared_curr,
			               shared_total,
			               std::move(arg_extra_mix.getValue()),
			               std::move(arg_extra_delim.getValue())));
	}
	catch(TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}

