#include <ipv4_generator.hpp>
#include <gettext.h>
#include <logging.h>
#include <cstdio>
#include <cstdlib>
#include <memory.h>

namespace cursor {

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

IPv4Generator::IPv4Generator(const bool repeat,
                             const bool mix /*= false*/)
	: repeat_(repeat)
	, mix_(mix)
	, size_(0)
	, counter_(0)
	, prime_number_(997)
	, initial_(0)
	, skip_private_(true)
	, shuffle_(NULL)
{
}

int
IPv4Generator::init(const char * init, const size_t initsz,
                    char * state, const size_t &statesz,
                    const uint32_t shuffle_restore_val /*= 0*/)
{
	counter_ = 0;
	char * init_s = new char[initsz + 1];
	memcpy(init_s, init, initsz);
	init_s[initsz] = 0;
	if(initsz == 0)
	{
		LOG(WARNING) << _("IPv4Generator: init is empty");
		state = NULL;
		return 0;
	}
	if (statesz < sizeof(sockaddr_in) + 1 + sizeof(sockaddr_in))
 	{
		LOG(ERROR) << _("IPv4Generator: state buffer is too short on init.");
		return -2;
	}
	size_t colon_pos = 0;
	size_t final_pos = 0;
	for(size_t pos = 0; pos < initsz; ++pos)
	{
		if(colon_pos == 0 && init_s[pos] == ':')
			colon_pos = pos;
		if(final_pos == 0 && init_s[pos] == '-')
			final_pos = pos;
	}

	memset(state, 0, sizeof(sockaddr_in)*2 + 1);
	state[0] = 0x02;
	sockaddr_in * addr = (sockaddr_in*)&state[1];
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("1.0.0.1");
	initial_ = ntohl(addr->sin_addr.s_addr);
	sockaddr_in * faddr = (sockaddr_in*)&state[1 + sizeof(sockaddr_in)];
	faddr->sin_family = AF_INET;
	faddr->sin_addr.s_addr = inet_addr("223.255.255.255");

	if(final_pos == 0)
	{
		LOG(WARNING) << _("IPv4Generator: no final value set.");
	}
	else if(colon_pos >= final_pos)
	{
		LOG(ERROR) << _("IPv4Generator: wrong final and colon positions.");
		init_s[final_pos] = 0;
	}
	else if(final_pos + 1 < initsz)
	{
		if(inet_pton(AF_INET, &init_s[final_pos + 1], &faddr->sin_addr) <= 0)
		{
			LOG(ERROR) << _("IPv4Generator: inet_pton with final error.")
				<< _("Message") << ": " << strerror(errno);
		}
		init_s[final_pos] = 0;
	}
	if(colon_pos != 0)
	{
		addr->sin_port = htons((unsigned short)atoi(&init_s[colon_pos+1]));
		init_s[colon_pos] = 0;
	}
	else
	{
		addr->sin_port = 0;
	}
	if(inet_pton(AF_INET, init_s, &addr->sin_addr) <= 0)
	{
		LOG(ERROR) << _("IPv4Generator: inet_pton error.") << " "
			<< _("Message") << ": " << strerror(errno);
		return 0;
	}
	initial_ = ntohl(addr->sin_addr.s_addr);
	delete [] init_s;

	uint32_t i_faddr  = ntohl(faddr->sin_addr.s_addr);
	uint32_t i_addr = ntohl(addr->sin_addr.s_addr);
	if(i_faddr >= i_addr)
	{
		size_ = i_faddr - i_addr;
		shuffle_.reset(new Shufor(size_, 0xabcdefed));
		if (shuffle_restore_val > 0)
		{
			if (shuffle_restore_val - initial_ > size_)
			{
				LOG(ERROR) << _("Wrong restore value.");
				return 0;
			}
			addr->sin_addr.s_addr = htonl(shuffle_restore_val);
			shuffle_->RestoreVal(0xabcdefed, size_, shuffle_restore_val - initial_);
			shift_to_next_host(addr, faddr);
		}
	}
	else
	{
		state[0] = 0x03;
		return 1;
	}


	if(shift_bad_addr(addr, faddr) < 0)
	{
		state[0] = 0x03;
		return 1;
	}

	return 2*sizeof(sockaddr_in) + 1;
}

IPv4Generator::~IPv4Generator()
{
}

int IPv4Generator::next(uint32_t &curr, const uint32_t final)
{
	++counter_;
	if(mix_)
	{
		if(shuffle_->IsCycle() && !repeat_)
			return -1;
		curr = htonl(initial_ + shuffle_->GetNext());
	}
	else
	{
		if(ntohl(curr) >= ntohl(final))
		{
			if(repeat_)
			{
				curr = inet_addr("1.0.0.1");
				counter_ = 0;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			curr = htonl(ntohl(curr) + 1);
		}
	}
	return 0;
}

typedef enum {
	IPv4_NETTYPE_A         = 1,
	IPv4_NETTYPE_B         = 2,
	IPv4_NETTYPE_C         = 3,
	IPv4_NETTYPE_LOCAL     = 4,
	IPv4_NETTYPE_UNKNOWN   = 0
} IPv4NetType;

typedef enum {
	IPv4_ADDRTYPE_RESERVED       = 1,
	IPv4_ADDRTYPE_BROADCAST      = 2,
	IPv4_ADDRTYPE_HOST           = 3,
	IPv4_ADDRTYPE_HOST_PRIVATE   = 4,
	IPv4_ADDRTYPE_NET            = 5,
	IPv4_ADDRTYPE_NET_PRIVATE    = 6,
	IPv4_ADDRTYPE_UNKNOWN        = 0
} IPv4AddrType;

typedef struct
{
	IPv4NetType  net_type;
	IPv4AddrType addr_type;
} IPv4Info;

/**@brief get ipv4 type
 * @param ip - uint32_t ip representation in network byte order*/
IPv4Info GetIPv4Info(const uint32_t ip)
{
	IPv4Info result;
	uint32_t hostip = ntohl(ip);
	uint32_t netid = 0, hostid = 0;
	int priv = 0;

	const uint32_t b1000 = (uint32_t)1  << 31; // 10000000 00000000 00000000 00000000
	const uint32_t b1100 = (uint32_t)3  << 30; // 11000000 00000000 00000000 00000000 
	const uint32_t b1110 = (uint32_t)7  << 29; // 11100000 00000000 00000000 00000000 
	const uint32_t b1111 = (uint32_t)15 << 28; // 11110000 00000000 00000000 00000000 

	const uint32_t anetmask  = (uint32_t)0x7f000000; // 01111111 00000000 00000000 00000000
	const uint32_t ahostmask = (uint32_t)0x00ffffff; // 00000000 11111111 11111111 11111111
	const uint32_t bnetmask  = (uint32_t)0x3fff0000; // 00111111 11111111 00000000 00000000
	const uint32_t bhostmask = (uint32_t)0x0000ffff; // 00000000 00000000 11111111 11111111
	const uint8_t  b1st_private = (uint8_t)0x3f & (uint8_t)172;  // 1-st byte of net id for private B networks
	const uint32_t cnetmask  = (uint32_t)0x1fffff00; // 00011111 11111111 11111111 00000000
	const uint32_t chostmask = (uint32_t)0x000000ff; // 00000000 00000000 00000000 11111111
	const uint8_t  c1st_private = (uint8_t)0x1f & (uint8_t)192; // 1-st byte of net id for private C networks

	result.net_type = IPv4_NETTYPE_UNKNOWN;
	result.addr_type = IPv4_ADDRTYPE_UNKNOWN; 
	
	if( (hostip & b1111) == b1110 )
	{
		result.addr_type = IPv4_ADDRTYPE_BROADCAST;
		return result;
	}
	if( (hostip & b1111) == b1111 )
	{
		result.addr_type = IPv4_ADDRTYPE_RESERVED;
		return result;
	}

	if((hostip & b1000) == 0)
	{
		netid  = (hostip & anetmask)/0x1000000;
		if(netid == 0)
			return result;
		hostid = (hostip & ahostmask);
		if( (hostip & anetmask) == anetmask )
			result.net_type = IPv4_NETTYPE_LOCAL;
		else
			result.net_type = IPv4_NETTYPE_A;
		if( hostid == ahostmask )
		{
			result.addr_type = IPv4_ADDRTYPE_BROADCAST;
			return result;
		}
		if(netid == 10)
			priv = 1;
	}
	else if((hostip & b1100) == b1000)
	{
		result.net_type = IPv4_NETTYPE_B;
		netid  = (hostip & bnetmask)/0x10000;
		hostid = (hostip & bhostmask);
		if( hostid == bhostmask )
		{
			result.addr_type = IPv4_ADDRTYPE_BROADCAST;
			return result;
		}
		if(  netid/0x100 == b1st_private && 16 <= netid%0x100 && netid%0x100 <= 31)
			priv = 1;
	}
	else if((hostip & b1110) == b1100)
	{
		result.net_type = IPv4_NETTYPE_C;
		netid  = (hostip & cnetmask);
		netid = netid/0x100;
		hostid = (hostip & chostmask);
		if( hostid == chostmask )
		{
			result.addr_type = IPv4_ADDRTYPE_BROADCAST;
			return result;
		}
		if(  netid/0x10000 == c1st_private && (netid%0x10000)/0x100 == 0xa8 )
			priv = 1;
	}
	if(hostid == 0)
	{
		if(priv)
			result.addr_type = IPv4_ADDRTYPE_NET_PRIVATE;
		else
			result.addr_type = IPv4_ADDRTYPE_NET;
	}
	else
	{
		if(priv)
			result.addr_type = IPv4_ADDRTYPE_HOST_PRIVATE;
		else
			result.addr_type = IPv4_ADDRTYPE_HOST;
	}
	return result;
}

/**@brief shift addr to first, which is HOST & NOT LOCAL & NOT UNKNONWN
 * @repeat - start from the first whe reach the upper bound */
int IPv4Generator::shift_bad_addr(struct sockaddr_in* addr, const struct sockaddr_in* faddr)
{
	IPv4Info addr_info;
	addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	while(addr_info.addr_type != IPv4_ADDRTYPE_HOST
		|| addr_info.net_type == IPv4_NETTYPE_LOCAL
		|| addr_info.net_type == IPv4_NETTYPE_UNKNOWN)
	{
		if((addr_info.addr_type == IPv4_ADDRTYPE_HOST_PRIVATE
				|| addr_info.addr_type == IPv4_ADDRTYPE_NET_PRIVATE))
		{
			if(!skip_private_)
				break;
			if(mix_)
			{
				if( next(addr->sin_addr.s_addr, faddr->sin_addr.s_addr) < 0 )
					return -1;
			}
			else
			{
				uint32_t i_addr_old = ntohl(addr->sin_addr.s_addr);
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
					default:
						LOG(ERROR) << _("Unknown private net type");
						return -2;
				};
				counter_ += ntohl(addr->sin_addr.s_addr) - i_addr_old;
			}
		}
		else
		{
			if( next(addr->sin_addr.s_addr, faddr->sin_addr.s_addr) < 0 )
				return -1;
		}
		addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	}
	return 0;
}

/**@beief Same as shoft_to_host, but shifts address to next first*/
int IPv4Generator::shift_to_next_host(struct sockaddr_in* addr, const struct sockaddr_in* faddr)
{
	if( next(addr->sin_addr.s_addr, faddr->sin_addr.s_addr) < 0 )
		return -1;
	return shift_bad_addr(addr, faddr);
}

/**@brief produces ipv4 sequence (with port, if set)
 *
 * example:
 *   "12.168.1.1:25"
 *   "12.168.1.2:25"
 *    ...
 * Note that ip, that can't be set to a host in Internet would be skipped
 * (local, broadcast, reserved, ...).
 *
 * state structure:
 * +---+---+...+---+---+...+---+
 * |STS|ADDR       |FADDR      |
 * +---+---+...+---+---+...+---+
 *
 * STS - status (0x03 - end of sequence, 0x02 - ADDR has valid value)
 * ADDR - next generated value
 * FADDR - boundary
 * */
IPv4Generator& IPv4Generator::operator()(char* state, size_t* statesz, size_t statemaxsz,
                                   char * next, size_t* nextsz, size_t nextmaxsz,
                                   int repeat)
{
	struct sockaddr_in* addr, *faddr;
	if(  state == NULL || statesz == NULL || statemaxsz == 0
	  || next  == NULL || nextsz  == NULL || nextmaxsz  == 0
	  || *nextsz > nextmaxsz || *statesz > statemaxsz
	  )
	{
		LOG(ERROR) << _("ipv4_generator: wrong args.");
		*nextsz = 0;
		return *this;
	}

	// end of sequence
	if(state[0] == 0x03)
	{
		*nextsz = 0;
		return *this;
	}

	if(*statesz == 0)
	{
		if(statemaxsz < sizeof(sockaddr_in) + 1 + sizeof(sockaddr_in))
 		{
			LOG(ERROR) << _("ipv4_generator: state buffer is too short.");
			*nextsz = 0;
			return *this;
		}
		memset(state, 0, statemaxsz);
		state[0] = 0x02;
		addr = (sockaddr_in*)&state[1];
		addr->sin_family = AF_INET;
		addr->sin_addr.s_addr = inet_addr("1.0.0.1");
		faddr = (sockaddr_in*)&state[1 + sizeof(sockaddr_in)];
		faddr->sin_family = AF_INET;
		faddr->sin_addr.s_addr = inet_addr("223.255.255.255");
	}
	else if(state[0] == 0x02)
	{
		if(*statesz < sizeof(sockaddr_in) + 1 + sizeof(sockaddr_in))
 		{
			LOG(ERROR) << _("ipv4_generator: state is too short.");
			*nextsz = 0;
			return *this;
		}
		addr = (sockaddr_in*)&state[1];
		faddr = (sockaddr_in*)&state[1 + sizeof(sockaddr_in)];
	}
	else
	{
		LOG(ERROR) << _("wrong ipv4_generator state structure");
		state[0] = 0x03;
		*statesz = 1;
		*nextsz = 0;
		return *this;
	}
	*statesz = 1 + sizeof(sockaddr_in) + sizeof(sockaddr_in);

	char tmp[60];
	memset(tmp, 0, sizeof(tmp));
	if(inet_ntop(AF_INET, &addr->sin_addr, tmp, sizeof(tmp) - 10) == NULL)
	{
		*nextsz = 0;
		return *this;
	}
	if(addr->sin_port != 0)
		sprintf(&tmp[strlen(tmp)], ":%u", ntohs(addr->sin_port));
	if(nextmaxsz < strlen(tmp))
	{
		LOG(ERROR) << _("ipv4_generator: output buffer is too small.");
		nextsz = 0;
		return *this;
	}
	memset(next, 0, nextmaxsz);
	memcpy(next, tmp, strlen(tmp));
	*nextsz = strlen(tmp);

	if( shift_to_next_host(addr, faddr) < 0)
	{
		state[0] = 0x03;
		*statesz = 1;
		return *this;
	}
}

} // namespace

