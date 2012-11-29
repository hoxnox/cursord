/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_generator.hpp"
#include <limits>
#include <cstring>

namespace cursor {


///////////////////////////////////////////////////////////////////////////
// generators

/**@var CursorGenerator::generator
 * @brief Generator itself
 *
 * Generator is C/C++ function with the following signature:
 *
 * @code
 *      void (*generator)(char* state, size_t* statesz, const size_t statemaxsz,
 *                        char * next, size_t* nextsz, const size_t nextmaxsz, 
 *                        const int repeat);
 * @endcode
 *
 * state - is a store of data, used to generate next value.
 *         Note that init argument of the cursor
 *         application is written to the state, so on first call of this
 *         function state can be not empty. Generator should write data for next
 *         call.
 * statesz - the size of the state parameter
 * statemaxsz - buffer size of the state parameter. The generator MUST NOT write
 *         over statemaxsz to the state.
 * next - is the buffer, used to save generated value.
 * nextsz - size of next parameter
 * nextmaxsz - maximum buffer size of the next parameter. 
 * repeat - Repeat flag is used, to indicate behaviour if the generator reaches
 *         maximum value (if it has).
 *
 * If generator can't produce new value, it should set nextsz to 0.
 *
 * To write your own generator, see examples below.*/


/**@brief produces <init>, <init>+1, ...*/
void simple_uint_generator(char* state, size_t* statesz, const size_t statemaxsz,
                           char * next, size_t* nextsz, const size_t nextmaxsz,
                           const int repeat)
{
	unsigned int init = 0;
	unsigned int nextv = 0;
	if(  state == NULL || statesz == NULL || statemaxsz == 0
	  || next  == NULL || nextsz  == NULL || nextmaxsz  == 0
	  || *nextsz > nextmaxsz || *statesz > statemaxsz
	  )
	{
		LOG(ERROR) << _("simple_uint_generator: wrong args.");
		*nextsz = 0;
		return;
	}
	if(*statesz == 0)
		init = 0;
	else
	{
		char * init_c = (char*)malloc(*statesz + 1);
		memset(init_c, 0, *statesz + 1);
		memcpy(init_c, state, *statesz);
		init = (unsigned)atoi(init_c);
		free(init_c);
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
	if(strlen(tmp) > nextmaxsz)
	{
		LOG(ERROR) << _("simple_uint_generator: next buffer is too small.");
		nextsz = 0;
		return;
	}
	*nextsz = strlen(tmp);
	if(statemaxsz < *nextsz + 1)
	{
		LOG(ERROR) << _("simple_uint_generator: state buffer is too small.");
		*nextsz = 0;
		return;
	}
	*statesz = *nextsz;
	memcpy(next, tmp, *nextsz);
	memcpy(state, tmp, *statesz);
}

/**@brief produces ipv4 sequence (with port, if set)
 *
 * example:
 *   "192.168.1.1:25"
 *   "192.168.1.2:25"
 *    ...
 * Note that ip, that can't be set to a host in Internet would be skipped
 * (local, broadcast, reserved, ...) 
void ipv4_generator(char* state, size_t* statesz, size_t* statemaxsz,
                    char * next, size_t* nextsz, size_t* nextmaxsz,
                    int repeat)
{
	size_t colon_pos = 1;
	struct sockaddr_in addr;
	whie(state[colon_pos] != ':' && colon_pos < *statesz)
		++colon_pos;
	// we don't need to convert in network byte order
	addr.sin_port = (unsigned short)atoi(&state[colon_pos+1]);
	state[colon_pos] = '\0';
	*statesz = colon_pos;
	if(inet_pton(AF_INET, &state[1], (struct sockaddr*)addr) <= 0)
	{
		LOG(ERROR) << _("ipv4_generator: inet_pton error.") << " "
			<< _("Message") << ": " << strerror(errno);
		nextsz = 0;
		return;
	}
	if(addr.sin_addr.s_addr == inet_addr("223.255.255.255"))
		addr.sin_addr.s_addr = inet_addr("1.0.0.1");

} TODO:*/

///////////////////////////////////////////////////////////////////////////
// CursorGenerator

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
	 ,repeat_(0)
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
			repeat_ = 1;
		}
		else if(arg.first == "init")
		{
			std::string init_utf8 = arg.second.toUTF8();
			if(init_utf8.length()  > statemaxsz_)
			{
				statemaxsz_ = init_utf8.length();
				state_ = (char*)realloc(state_, statemaxsz_);
				memset(state_, 0, statemaxsz_);
			}
			memcpy(state_, init_utf8.data(), init_utf8.length());
			statesz_ = init_utf8.length();
			initial_ = arg.second;
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
	size_t i = 0;
	if(!initial_.empty())
	{
		buf.push_back(initial_);
		initial_.clear();
		++i;
	}
	for(; i < count; ++i)
	{
		generator(state_, &statesz_, statemaxsz_, 
		          nextbuf_, &nextbufsz_, nextbufmaxsz_,
		          repeat_);
		if(nextbufsz_ == 0)
			break;
		buf.push_back(nx::String(nextbuf_, nextbuf_ + nextbufsz_));
	}
	return 0;
}

} // namespace

