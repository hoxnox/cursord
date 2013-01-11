/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_file.hpp"

namespace cursor {

CursorFile::CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
	 ,repeat_(false)
{
	size_t init = 0;
	nx::String fname;
	for(auto arg=args.begin(); arg != args.end(); ++arg )
	{
		nx::String tmp = arg->second;
		tmp.trim();
		if(arg->first == "init")
		{
			init = arg->second.toNumber();
		}
		else if(arg->first == "repeat" && tmp.trim().toLower() != L"false"
				&& tmp.trim().toLower() != L"0")
		{
			repeat_ = true;
		}
		else if(arg->first == "name")
		{
			fname = arg->second;
		}
		else
		{
			LOG(WARNING) << ("Unsupported argument.") << " "
				<< arg->first << " = " << arg->second;
		}
	}
	if(fname.empty())
	{
		LOG(ERROR) << _("File name is not set.");
		return;
	}
#ifdef MSVC
	file_.open(fname, std::ios::binary);
#else
	file_.open(fname.toUTF8(), std::ios::binary);
#endif
	if(!file_.is_open())
	{
		LOG(ERROR) << _("Error opening file.") << " "
			<<_("Filename") << ": " << fname.toUTF8();
		return;
	}
	std::string line;
	getline(file_, line);
	if(file_.eof() && line.empty())
	{
		LOG(WARNING) << _("File") << ": \"" << fname.toUTF8() << "\"" << " " 
			<< _("is empty.");
		repeat_ = false;
		return;
	}
	file_.clear();
	file_.seekg(0, std::ios::beg);
	if(init > 0)
		for(size_t i = 0; i < init - 1; ++i)
			getline(file_, line);
	LOG(INFO) << _("Starting file cursor.") << " "
		<<_("Filename") << ": \"" << fname.toUTF8() << "\""
		<< ", initial line: " << init;
}

CursorFile::~CursorFile()
{
	if(file_.is_open())
		file_.close();
}

int CursorFile::Next(const size_t count, std::deque<nx::String>& buf /*= buf_*/)
{
	for(size_t i = 0; i < count; ++i)
	{
		std::string line;
		getline(file_, line);
		if(file_.eof())
		{
			if(repeat_)
			{
				file_.clear();
				file_.seekg(0, std::ios::beg);
			}
			else
			{
				return 0;
			}
		}
		if(!line.empty())
			buf.push_back(nx::String::fromUTF8(line));
	}
	return 0;
}

} // namespace

