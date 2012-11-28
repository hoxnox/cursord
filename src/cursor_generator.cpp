/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_generator.hpp"
#include <limits>
#include <cstring>

namespace cursor {


///////////////////////////////////////////////////////////////////////////
// generators

void simple_uint_generator(char* state, size_t* statesz, size_t* statemaxsz,
                           char * next, size_t* nextsz, size_t* nextmaxsz)
{
	unsigned int init = 0;
	unsigned int nextv = 0;
	bool repeat = false;
	if(  state == NULL || statesz == NULL || statemaxsz == NULL
	  || next  == NULL || nextsz  == NULL || nextmaxsz  == NULL
	  || *nextsz > *nextmaxsz || *statesz > *statemaxsz
	  )
	{
		LOG(ERROR) << _("simple_uint_generator: wrong args.");
		*nextsz = 0;
		return;
	}
	if(*statesz == 0)
		init = 0;
	else if(state[0] == 'R')
	{
		repeat = true;
		if(*statesz > 1)
		{
			char * init_c = (char*)malloc(*statesz);
			memset(init_c, 0, *statesz);
			memcpy(init_c, &state[1], *statesz - 1);
			init = (unsigned)atoi(init_c);
			free(init_c);
		}
	}
	if(init == std::numeric_limits<unsigned int>::max())
	{
		if(repeat)
		{
			nextv = 0;
		}
		else
		{
			LOG(INFO) << _("simple_uint_generator: reached maximum value.");
			*nextsz = 0;
			return;
		}
	}
	else
	{
		nextv = ++init;
	}
	const int tmpsz = std::numeric_limits<unsigned int>::digits10 + 1;
	char *tmp = (char *)malloc(tmpsz);
	memset(tmp, 0, tmpsz);

	sprintf(tmp, "%d", nextv);
	if(strlen(tmp) > *nextmaxsz)
	{
		LOG(ERROR) << _("simple_uint_generator: next buffer is too small.");
		nextsz = 0;
		return;
	}
	*nextsz = strlen(tmp);
	if(*statemaxsz < *nextsz + 1)
	{
		LOG(ERROR) << _("simple_uint_generator: state buffer is too small.");
		*nextsz = 0;
		return;
	}
	*statesz = *nextsz + 1;
	memcpy(next, tmp, *nextsz);
	memcpy(&state[1], tmp, *nextsz);
}

inline long max(const long lhv, const long rhv)
{
	return lhv > rhv ? lhv : rhv;
}

CursorGenerator::CursorGenerator(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
	 ,nextbufsz_(0)
	 ,nextbufmaxsz_(2048)
	 ,statesz_(0)
	 ,statemaxsz_(2048)
	 ,generator(NULL)
{
	state_   = (char *)malloc(statemaxsz_);
	memset(state_, 0, statemaxsz_);
	nextbuf_ = (char *)malloc(nextbufmaxsz_);
	memset(nextbuf_, 0, nextbufmaxsz_);
	for(auto arg : args)
	{
		if(arg.first == "repeat" && arg.second.trim().toLower() != L"false"
				&& arg.second.trim().toLower() != L"0")
		{
			state_[0] = 'R';
			statesz_ = max(1, statesz_);
		}
		else if(arg.first == "init")
		{
			std::string init_utf8 = arg.second.toUTF8();
			if(init_utf8.length() + 1  > statemaxsz_)
			{
				statemaxsz_ = init_utf8.length() + 1;
				state_ = (char*)realloc(state_, statemaxsz_);
				memset(state_, 0, statemaxsz_);
			}
			memcpy(&state_[1], init_utf8.data(), init_utf8.length());
		}
		else if(arg.first == "name")
		{
			nx::String name = arg.second.trim().toLower();
			if(name == "int")
			{
				generator = simple_uint_generator;
			}
			else
			{
				LOG(ERROR) << _("Unsupported generator.") << " "
					<< arg.second.toUTF8();
			}
		}
		else
		{
			LOG(WARNING) << ("Unsupported argument.") << " "
				<< arg.first << " = " << arg.second;
		}
	}
	if(generator == NULL)
	{
		LOG(ERROR) << _("Generator name is not set.");
		return;
	}
}

CursorGenerator::~CursorGenerator()
{
}

int CursorGenerator::Next(const size_t count, std::deque<nx::String>& buf)
{
	if(generator == NULL)
		return 0;
	for(size_t i = 0; i < count; ++i)
	{
		generator(state_, &statesz_, &statemaxsz_, nextbuf_, &nextbufsz_, &nextbufmaxsz_);
		if(nextbufsz_ == 0)
			break;
		buf.push_back(nx::String(nextbuf_, nextbuf_ + nextbufsz_));
	}
	return 0;
}

} // namespace

