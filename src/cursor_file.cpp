/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_file.hpp"

namespace cursor {

std::string CursorFile::getinfo(size_t init /*=""*/)
{
	std::stringstream info;
	info << _("Starting file cursor.");
	info << " " <<  _("Filename") << ": \"" << fname_.toUTF8() << "\"";
	if(ftype_ == FTYPE_TEXT)
		info << ", filetype TEXT";
	if(ftype_ == FTYPE_IPv4)
		info << ", filetype IPv4";
	if(!prefix_.empty())
		info << ", prefix: " << prefix_.toUTF8();
	if(!suffix_.empty())
		info << ", suffix: " << suffix_.toUTF8();
	if(init != 0)
		info << ", initial: " << init;
	return info.str();
}

CursorFile::CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
	 ,repeat_(false)
	 ,ftype_(FTYPE_TEXT)
	, ipv4gen_(false)
	, statesz_(0)
	, initialized_(false)
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
			if(ftype.toLower() == "text")
				ftype_ = FTYPE_TEXT;
			else if(ftype.toLower() == "ipv4")
				ftype_ = FTYPE_IPv4;
			else if(ftype.toLower() == "ipv4rg")
				ftype_ = FTYPE_IPv4RANGES;
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
			if(getnext().empty())
			{
				LOG(WARNING) << "Error skipping initial value";
				break;
			}
	LOG(INFO) << getinfo(init);
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
		if(file_.eof() && repeat_)
		{
			file_.clear();
			file_.seekg(0, std::ios::beg);
		}
		else
		{
			return result;
		}
	if(ftype_ == FTYPE_TEXT)
		getline(file_, result);
	else if(ftype_ == FTYPE_IPv4)
	{
		char ip[4];
		char buf[50];
		memset(buf, 0, sizeof(buf));
		file_.read(ip, 4);
		if(!file_.good())
		{
			if(file_.eof() && repeat_)
			{
				file_.clear();
				file_.seekg(0, std::ios::beg);
				file_.read(ip, 4);
				if(!file_.good())
					return result;
			}
			else
			{
				return result;
			}
		}
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_addr.s_addr = *reinterpret_cast<uint32_t*>(ip);
		if(inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf)) == NULL)
			LOG(WARNING) << _("Error converting ip to char.") << " "
				<< _("Message") << ": " << strerror(errno);
		else
			result = buf;
	}
	else if(ftype_ == FTYPE_IPv4RANGES)
	{
		char rsbuf[50];
		size_t rsbufsz = 0;
		if(initialized_)
			ipv4gen_(state_, &statesz_, sizeof(state_),
					rsbuf, &rsbufsz, sizeof(rsbuf), 0);
		while( rsbufsz == 0 || !initialized_)
		{
			char ipbuf[8];
			char buf[100];
			memset(buf, 0, sizeof(buf));
			file_.read(ipbuf, 8);
			if(!file_.good())
			{
				if(file_.eof() && repeat_)
				{
					file_.clear();
					file_.seekg(0, std::ios::beg);
					file_.read(ipbuf, 8);
					if(!file_.good())
						return result;
				}
				else
				{
					return result;
				}
			}
			struct sockaddr_in addr1, addr2;
			size_t suffpos = 0;
			memset(&addr1, 0, sizeof(addr1));
			memset(&addr2, 0, sizeof(addr2));
			addr1.sin_addr.s_addr = *reinterpret_cast<uint32_t*>(ipbuf);
			addr2.sin_addr.s_addr = *reinterpret_cast<uint32_t*>(&ipbuf[4]);
			if(inet_ntop(AF_INET, &addr1.sin_addr, buf, sizeof(buf)/2) == NULL)
			{
				LOG(WARNING) << _("Error converting ip to char.") << " "
					<< _("Message") << ": " << strerror(errno);
			}
			else
			{
				size_t ad1ln = strlen(buf);
				if(inet_ntop(AF_INET, &addr2.sin_addr, &buf[ad1ln + 1], 
							sizeof(buf) - ad1ln - 1) == NULL)
				{
					LOG(WARNING) << _("Error converting ip to char.") << " "
						<< _("Message") << ": " << strerror(errno);
				}
				else
				{
					buf[ad1ln] = '-';
					memset(state_, 0, sizeof(state_));
					statesz_ = ipv4gen_.init(buf, strlen(buf), 
							state_, sizeof(state_));
					initialized_ = true;
					ipv4gen_(state_, &statesz_, sizeof(state_),
						rsbuf, &rsbufsz, sizeof(rsbuf), 0);
				}
			}
		}
		if(rsbufsz > 0)
			result.assign(rsbuf);
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

