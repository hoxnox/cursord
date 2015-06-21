/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_file.hpp"
#include <endians.hpp>
#include <ipv4.hpp>


namespace cursor {

std::string CursorFile::getinfo(size_t init /*=""*/)
{
	std::stringstream info;
	info << _("Starting file cursor.");
	info << " " <<  _("Filename") << ": \"" << fname_.toUTF8() << "\"";
	if(ftype_ == FTYPE_TEXT)
		info << ", filetype TEXT";
	if(ftype_ == FTYPE_CIDR)
		info << ", filetype CIDR";
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

size_t getFileSz(std::ifstream& file, CursorFile::FileTypes type)
{
	size_t cnt = 0;
	if(!file.is_open())
		return 0;
	file.clear();
	file.seekg(0, std::ios::beg);
	if (type == CursorFile::FTYPE_IPv4 || type == CursorFile::FTYPE_IPv4RANGES)
	{
		size_t cnt = 0;
		file.seekg (0, file.end);
		size_t filesz = file.tellg();
		file.seekg(0, std::ios::beg);
		if(type == CursorFile::FTYPE_IPv4RANGES)
		{
			for(size_t i = 0; i < filesz/8 && !file.eof(); ++i)
			{
				char buf[8] = {0,0,0,0,0,0,0,0};
				file.read(buf, 8);
				uint32_t rha = ntohl(*(uint32_t*)&buf[4]);
				uint32_t lha = ntohl(*(uint32_t*)buf);
				if(rha > lha)
					cnt += (rha - lha) + 1;
			}
		}
		else
			cnt = filesz/4;
	}
	else if (type == CursorFile::FTYPE_TEXT || type == CursorFile::FTYPE_CIDR)
	{
		size_t line_cnt = 1;
		std::string line;
		getline(file, line);
		while(!file.eof())
		{
			if (type == CursorFile::FTYPE_CIDR)
			{
				std::pair<uint32_t, uint32_t> range = nx::CIDRtoIPv4Range(line);
				if (range.first == 0 && range.second == 0)
				{
					LOG(WARNING) << _("CursorFile: wrong CIDR value.")
					             << _(" Line: ") << line_cnt
					             << _(" Value: ") << line;
					continue;
				}
				if (range.second < range.first)
				{
					LOG(WARNING) << _("CursorFile: CIDRtoIPv4Range error.")
					             << _(" Line: ") << line_cnt
					             << _(" Value: ") << line
					             << _(" Range first: ") << range.first
					             << _(" Range second: ") << range.second;
					continue;
				}
				if (range.second == range.first)
					cnt += 1;
				else if (range.second - range.first > 2)
					cnt += ntohl(range.second) - ntohl(range.first) - 2;
			}
			else if (type == CursorFile::FTYPE_TEXT)
			{
				cnt = line_cnt;
			}
			++line_cnt;
			getline(file, line);
		}
	}
	else
	{
		LOG(ERROR) << _("Unknown file type.")
		           << _(" Type: ") << type;
		return 0;
	}
	file.clear();
	file.seekg(0, std::ios::beg);
	return cnt;
}

CursorFile::CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
	, repeat_(false)
	, ftype_(FTYPE_TEXT)
	, ipv4gen_(false)
	, statesz_(0)
	, initialized_(false)
	, totalsz_(0)
	, passedsz_(0)
{
	init(addr, args);
}

CursorFile::CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args,
                       const size_t shared_curr, const size_t shared_total)
	: Cursor(addr, shared_curr, shared_total)
	, repeat_(false)
	, ftype_(FTYPE_TEXT)
	, ipv4gen_(false)
	, statesz_(0)
	, initialized_(false)
	, totalsz_(0)
	, passedsz_(0)
{
	init(addr, args);
}

void CursorFile::init(const Cursor::Sockaddr addr, const Cursor::Args args)
{
	size_t init = 0;
	nx::String ftype;
	for(std::map<nx::String, nx::String>::const_iterator arg=args.begin();
			arg != args.end(); ++arg )
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
			else if(ftype.toLower() == "cidr")
				ftype_ = FTYPE_CIDR;
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
	file_.open(fname_.toUTF8().c_str(), std::ios::binary);
#endif
	if(!file_.is_open())
	{
		LOG(ERROR) << _("Error opening file.") << " "
			<<_("Filename") << ": " << fname_.toUTF8();
		return;
	}
	totalsz_ = getFileSz(file_, ftype_);
	if(totalsz_ == 0)
	{
		LOG(WARNING) << _("File") << ": \"" << fname_.toUTF8() << "\"" << " " 
			<< _("is empty.");
		repeat_ = false;
		return;
	}
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

bool
CursorFile::init_ipv4gen(uint32_t ip_low, uint32_t ip_hi)
{
	struct sockaddr_in addr1, addr2;
	size_t suffpos = 0;
	memset(&addr1, 0, sizeof(addr1));
	memset(&addr2, 0, sizeof(addr2));
	addr1.sin_addr.s_addr = ip_low;
	addr2.sin_addr.s_addr = ip_hi;
	char buf[100];
	memset(buf, 0, sizeof(buf));
	if(inet_ntop(AF_INET, &addr1.sin_addr, buf, sizeof(buf)/2) == NULL)
	{
		LOG(WARNING) << _("Error converting ip to char.") << " "
			<< _("Message") << ": " << strerror(errno);
		return false;
	}
	size_t ad1ln = strlen(buf);
	if(inet_ntop(AF_INET, &addr2.sin_addr, &buf[ad1ln + 1], 
				sizeof(buf) - ad1ln - 1) == NULL)
	{
		LOG(WARNING) << _("Error converting ip to char.") << " "
			<< _("Message") << ": " << strerror(errno);
		return false;
	}
	buf[ad1ln] = '-';
	memset(state_, 0, sizeof(state_));
	statesz_ = ipv4gen_.init(buf, strlen(buf), 
			state_, sizeof(state_));
	return true;
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
	{
		getline(file_, result);
		++passedsz_;
	}
	if(ftype_ == FTYPE_CIDR)
	{
		char rsbuf[50];
		size_t rsbufsz = 0;
		if (initialized_)
		{
			ipv4gen_(state_, &statesz_, sizeof(state_),
					rsbuf, &rsbufsz, sizeof(rsbuf), 0);
			if (rsbufsz == 0)
				initialized_ = false;
		}
		while( rsbufsz == 0 || !initialized_)
		{
			std::string line;
			getline(file_, line);
			if(!file_.good())
			{
				if(file_.eof() && repeat_)
				{
					file_.clear();
					file_.seekg(0, std::ios::beg);
					getline(file_, line);
					if(!file_.good())
						return "";
				}
				else
				{
					return "";
				}
			}
			std::pair<uint32_t, uint32_t> range = nx::CIDRtoIPv4Range(line);
			if (range.first == 0 || range.second == 0 || range.second < range.first)
			{
				LOG(WARNING) << _("CursorFile: error CIDR converting.")
				             << _(" Value: ") << line;
				continue;
			}
			if (range.first == range.second)
			{
				struct sockaddr_in addr;
				memset(&addr, 0, sizeof(addr));
				addr.sin_addr.s_addr = range.first;
				memset(rsbuf, 0, sizeof(rsbuf));
				if (inet_ntop(AF_INET, &addr.sin_addr, rsbuf, sizeof(rsbuf)) == NULL)
				{
					LOG(WARNING) << _("Error converting ip to char.") << " "
						<< _("Message") << ": " << strerror(errno);
					continue;
				}
				rsbufsz = strlen(rsbuf);
				if (rsbufsz == 0) 
					LOG(WARNING) << _("CursotFile: inet_ntop returned zero string.");
				else
					break;
			}
			else if (range.second - range.first <= 2)
			{
				LOG(WARNING) << _("CursorFile: CIDR has no valid hosts.")
				             << _(" Value: ") << line;
				continue;
			}
			else if (init_ipv4gen(htonl(ntohl(range.first) + 1), htonl(ntohl(range.second) - 1)))
			{
				initialized_ = true;
				ipv4gen_(state_, &statesz_, sizeof(state_),
					rsbuf, &rsbufsz, sizeof(rsbuf), 0);
				passedsz_ += ipv4gen_.size();
			}
		}
		if(rsbufsz > 0)
			result.assign(rsbuf);
	}
	else if(ftype_ == FTYPE_IPv4)
	{
		char ip[4];
		char buf[50];
		memset(buf, 0, sizeof(buf));
		file_.read(ip, 4);
		++passedsz_;
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
		{
			ipv4gen_(state_, &statesz_, sizeof(state_),
					rsbuf, &rsbufsz, sizeof(rsbuf), 0);
		}
		while( rsbufsz == 0 || !initialized_)
		{
			char ipbuf[8];
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
			if(init_ipv4gen(*reinterpret_cast<uint32_t*>(ipbuf), *reinterpret_cast<uint32_t*>(&ipbuf[4])))
			{
				initialized_ = true;
				ipv4gen_(state_, &statesz_, sizeof(state_),
					rsbuf, &rsbufsz, sizeof(rsbuf), 0);
				passedsz_ += ipv4gen_.size();
			}
		}
		if(rsbufsz > 0)
			result.assign(rsbuf);
	}
	return result;
}

int CursorFile::do_next(const size_t count, std::deque<nx::String>& buf /*= buf_*/)
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
				break;
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
	if(!repeat_)
	{
		size_t passedsz = passedsz_;
		if(ftype_ == FTYPE_IPv4RANGES)
			passedsz -= ipv4gen_.size() - ipv4gen_.pos();
		int percent = ((float)passedsz/totalsz_)*100;
		LOG(INFO) << _("Progress") << ": " << passedsz << "/" << totalsz_
			<< " (" << percent << "%)";
	}
	return 0;
}

} // namespace

