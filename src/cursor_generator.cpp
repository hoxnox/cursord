/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_generator.hpp"
#include <limits>
#include <cstring>
#include <sstream>
#include <nx_socket.h>

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
 * state - is a store of data, used to generate next value. Note that initial
 *         value must be returned too. So one approach to generator architecture is to
 *         write "future value" to the state, rather than "current". Note also,
 *         that the value of the init argument is written to the state, so on
 *         first call of generator there may be initial value in the state.
 *         Sometimes this initial value need preprocessing before returning out.
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


/**@brief produces <init>, <init>+1, ...
 *
 * We will write next int to the state buffer. Note that initial value will be
 * in string representation! To distinct first call from others, we will write
 * '0x02' byte to the state[0]. If the end of sequence reached, '0x03' will be
 * written to the state[0].*/
void simple_uint_generator(char* state, size_t* statesz, const size_t statemaxsz,
                           char * next, size_t* nextsz, const size_t nextmaxsz,
                           const int repeat)
{
	unsigned int currv = 0;
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

	// end of sequence
	if(state[0] == 0x03)
	{
		*nextsz = 0;
		return;
	}

	if(statesz == 0)
	{
		currv = 0;
	}
	else if(state[0] == 0x02)
	{ // regular
		if(*statesz < sizeof(unsigned int) + 1)
		{
			LOG(ERROR) << _("simple_uint_generator: state is too short.");
			*nextsz = 0;
			return;
		}
		currv = *((unsigned int*)&state[1]);
	}
	else
	{ // initial
		currv = (unsigned int)atol(state);
	}

	const int tmpsz = std::numeric_limits<unsigned int>::digits10 + 1;
	char *tmp = (char *)malloc(tmpsz);
	memset(tmp, 0, tmpsz);
	sprintf(tmp, "%u", currv);
	if(nextmaxsz < strlen(tmp))
	{
		LOG(ERROR) << _("simple_uint_generator: output buffer is too smal.");
		*nextsz = 0;
		return;
	}
	memset(next, 0, nextmaxsz);
	memcpy(next, tmp, strlen(tmp));
	*nextsz = strlen(tmp);

	// prepare future
	if(currv == std::numeric_limits<unsigned int>::max())
	{
		if(repeat)
		{
			nextv = 0;
		}
		else
		{
			LOG(INFO) << _("simple_uint_generator: reached maximum value.");
			state[0] = 0x03;
			*statesz = 1;
			return;
		}
	}
	else
	{
		nextv = ++currv;
	}
	if(statemaxsz < sizeof(unsigned int) + 1)
	{
		LOG(ERROR) << _("simple_uint_generator: state buffer is too small.");
		*nextsz = 0;
		return;
	}
	memset(state, 0, statemaxsz);
	state[0] = 0x02;
	*((unsigned int*)&state[1]) = nextv;
	*statesz = sizeof(unsigned int) + 1;
}

/**@brief produces ipv4 sequence (with port, if set)
 *
 * example:
 *   "192.168.1.1:25"
 *   "192.168.1.2:25"
 *    ...
 * Note that ip, that can't be set to a host in Internet would be skipped
 * (local, broadcast, reserved, ...).*/
void ipv4_generator(char* state, size_t* statesz, size_t statemaxsz,
                    char * next, size_t* nextsz, size_t nextmaxsz,
                    int repeat)
{
	size_t colon_pos = 1;
	struct sockaddr_in* addr;
	memset(&addr, 0, sizeof(addr));
	if(  state == NULL || statesz == NULL || statemaxsz == 0
	  || next  == NULL || nextsz  == NULL || nextmaxsz  == 0
	  || *nextsz > nextmaxsz || *statesz > statemaxsz
	  )
	{
		LOG(ERROR) << _("ipv4_generator: wrong args.");
		*nextsz = 0;
		return;
	}

	// end of sequence
	if(state[0] == 0x03)
	{
		*nextsz = 0;
		return;
	}

	if(*statesz == 0)
	{
		if(statemaxsz < sizeof(sockaddr_in) + 1)
 		{
			LOG(ERROR) << _("ipv4_generator: state buffer is too short.");
			*nextsz = 0;
			return;
		}
		state[0] = 0x02;
		addr = (sockaddr_in*)&state[1];
		memset(addr, 0, sizeof(sockaddr_in));
		addr->sin_family = AF_INET;
		addr->sin_addr.s_addr = inet_addr("1.0.0.1");
	}
	else if(state[0] == 0x02)
	{ // regular
		if(*statesz < sizeof(sockaddr_in) + 1)
 		{
			LOG(ERROR) << _("ipv4_generator: state is too short.");
			*nextsz = 0;
			return;
		}
		addr = (sockaddr_in*)&state[1];
	}
	else
	{ // initial
		if(statemaxsz < sizeof(sockaddr_in) + 1)
 		{
			LOG(ERROR) << _("ipv4_generator: state buffer is too short.");
			*nextsz = 0;
			return;
		}
		while(state[colon_pos] != ':' && colon_pos < *statesz)
			++colon_pos;
		// we don't need to convert in network byte order
		char * state_cp = new char[*statesz];
		memset(state_cp, 0, *statesz);
		memcpy(state_cp, state, *statesz);
		state[0] = 0x02;
		addr = (sockaddr_in*)&state[1];
		addr->sin_port = (unsigned short)atoi(&state_cp[colon_pos+1]);
		state_cp[colon_pos] = '\0';
		if(inet_pton(AF_INET, state_cp, &addr->sin_addr) <= 0)
		{
			LOG(ERROR) << _("ipv4_generator: inet_pton error.") << " "
				<< _("Message") << ": " << strerror(errno);
			nextsz = 0;
			return;
		}
		delete [] state_cp;
	}
	*statesz = 1 + sizeof(sockaddr_in);

	IPv4Info addr_info;
	addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	if(addr_info.addr_type == IPv4_ADDRTYPE_HOST_PRIVATE)
	{
		switch(addr_info.net_type)
		{
			case IPv4_NETTYPE_A:
				addr->sin_addr.s_addr = inet_addr("11.0.0.1");
				break;
			case IPv4_NETTYPE_B: 
				addr->sin_addr.s_addr = inet_addr("172.32.0.1");
				break;
			case IPv4_NETTYPE_C:
				addr->sin_addr.s_addr = inet_addr("192.169.0.1");
				break;
		}
	}
	addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	while(addr_info.addr_type != IPv4_ADDRTYPE_HOST 
		|| addr_info.net_type == IPv4_NETTYPE_LOCAL 
		|| addr_info.net_type == IPv4_NETTYPE_UNKNOWN)
	{
		if(ntohl(addr->sin_addr.s_addr) >= ntohl(inet_addr("223.255.255.255")))
		{
			if(repeat)
			{       	
				addr->sin_addr.s_addr = inet_addr("1.0.0.1");
			}
			else
			{
				state[0] = 0x03;
				*statesz = 1;
				return;
			}
		}
		else
		{
			addr->sin_addr.s_addr = htonl(ntohl(addr->sin_addr.s_addr) + 1);
		}
		addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	}

	char tmp[50];
	memset(tmp, 0, sizeof(tmp));
	if(inet_ntop(AF_INET, &addr->sin_addr, tmp, sizeof(tmp) - 1) == NULL)
	{
		*nextsz = 0;
		return;
	}
	if(nextmaxsz < strlen(tmp))
	{
		LOG(ERROR) << _("ipv4_generator: output buffer is too small.");
		nextsz = 0;
		return;
	}
	memcpy(next, tmp, strlen(tmp));
	*nextsz = strlen(tmp);

	do{
		if(ntohl(addr->sin_addr.s_addr) >= ntohl(inet_addr("223.255.255.255")))
		{
			if(repeat)
			{       	
				addr->sin_addr.s_addr = inet_addr("1.0.0.1");
			}
			else
			{
				state[0] = 0x03;
				*statesz = 1;
				return;
			}
		}
		else
		{
			addr->sin_addr.s_addr = htonl(ntohl(addr->sin_addr.s_addr) + 1);
		}
		addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	} while(addr_info.addr_type != IPv4_ADDRTYPE_HOST 
		|| addr_info.net_type == IPv4_NETTYPE_LOCAL 
		|| addr_info.net_type == IPv4_NETTYPE_UNKNOWN);
}

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
		}
		else if(arg.first == "name")
		{
			nx::String name = arg.second.trim().toLower();
			if(name == "int")
			{
				generator = simple_uint_generator;
			}
			else if(name == "ipv4")
			{
				generator = ipv4_generator;
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
	if(state_)
		free(state_);
	if(nextbuf_)
		free(nextbuf_);
}

int CursorGenerator::Next(const size_t count, std::deque<nx::String>& buf)
{
	if(generator == NULL)
		return 0;
	size_t i = 0;
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

