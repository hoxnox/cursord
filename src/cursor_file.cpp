/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_file.hpp"

namespace cursor {

std::string CursorFile::getinfo(std::string init /*=""*/)
{
	std::string info = _("Starting file cursor.");
	info = info + " " +  _("Filename") + ": \"" + fname_.toUTF8() + "\"";
	if(ftype_ == FTYPE_TEXT)
		info += ", filetype TEXT";
	if(ftype_ == FTYPE_IPv4)
		info += ", filetype IPv4";
	if(!prefix_.empty())
		info += ", prefix: " + prefix_.toUTF8();
	if(!suffix_.empty())
		info += ", suffix: " + suffix_.toUTF8();
	if(!init.empty())
		info += ", initial line: " + init;
	return info;
}

CursorFile::CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
	 ,repeat_(false)
	 ,ftype_(FTYPE_TEXT)
{
	size_t init = 0;
	nx::String ftype;
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
			fname_ = arg->second;
		}
		else if(arg->first == "type")
		{
			ftype = arg->second;
			if(ftype.toLower() == "ipv4")
				ftype_ = FTYPE_IPv4;
			else
				LOG(WARNING) << _("Unsupported file type. Treat as TEXT.") << " "
					<< arg->first << " =  " << arg->second;
		}
		else if(arg->first == "prefix")
		{
			prefix_ = arg->second;
		}
		else if(arg->first == "suffix")
		{
			suffix_ = arg->second;
		}
		else
		{
			LOG(WARNING) << _("Unsupported argument.") << " "
				<< arg->first << " = " << arg->second;
		}
	}
	if(fname_.empty())
	{
		LOG(ERROR) << _("File name is not set.");
		return;
	}
#ifdef MSVC
	file_.open(fname_, std::ios::binary);
#else
	file_.open(fname_.toUTF8(), std::ios::binary);
#endif
	if(!file_.is_open())
	{
		LOG(ERROR) << _("Error opening file.") << " "
			<<_("Filename") << ": " << fname_.toUTF8();
		return;
	}
	std::string line;
	getline(file_, line);
	if(file_.eof() && line.empty())
	{
		LOG(WARNING) << _("File") << ": \"" << fname_.toUTF8() << "\"" << " " 
			<< _("is empty.");
		repeat_ = false;
		return;
	}
	file_.clear();
	file_.seekg(0, std::ios::beg);
	if(init > 0)
		for(size_t i = 0; i < init - 1; ++i)
			getnext();
	LOG(INFO) << getinfo();
}

CursorFile::~CursorFile()
{
	if(file_.is_open())
		file_.close();
}

std::string CursorFile::getnext()
{
	std::string result;
	if(!file_.good())
		return result;
	if(ftype_ == FTYPE_TEXT)
		getline(file_, result);
	else if(ftype_ == FTYPE_IPv4)
	{
		char ip[4];
		char buf[50];
		memset(buf, 0, sizeof(buf));
		file_.read(ip, 4);
		if(!file_.good())
			return result;
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_addr.s_addr = *reinterpret_cast<uint32_t*>(ip);
		if(inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf)) == NULL)
			LOG(WARNING) << _("Error converting ip to char.") << " "
				<< _("Message") << ": " << strerror(errno);
		else
			result = buf;
	}
	return result;
}

int CursorFile::Next(const size_t count, std::deque<nx::String>& buf /*= buf_*/)
{
	for(size_t i = 0; i < count; ++i)
	{
		std::string line;
		line = getnext();
		if(!file_.good())
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
		{
			nx::String result;
			if(!prefix_.empty())
				result = prefix_;
			result += nx::String::fromUTF8(line);
			if(!suffix_.empty())
				result += suffix_;
			buf.push_back(result);
		}
	}
	return 0;
}

} // namespace

