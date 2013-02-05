#include <ipv4_generator.hpp>
#include <gettext.h>
#include <logging.h>

namespace cursor {

IPv4Generator::IPv4Generator(const char * init,
                             const size_t initsz,
                             const bool repeat,
                             const bool mix /*= false*/)
	: repeat_(repeat)
	, mix_(mix)
{
}

IPv4Generator::~IPv4Generator()
{
}

/**@brief shift addr to first, which is HOST & NOT LOCAL & NOT UNKNONWN
 * @repeat - start from the first whe reach the upper bound */
int IPv4Generator::shift_to_host(struct sockaddr_in* addr, struct sockaddr_in* faddr)
{
	IPv4Info addr_info;
	addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	while(addr_info.addr_type != IPv4_ADDRTYPE_HOST
		|| addr_info.net_type == IPv4_NETTYPE_LOCAL
		|| addr_info.net_type == IPv4_NETTYPE_UNKNOWN)
	{
		if(ntohl(addr->sin_addr.s_addr) >= ntohl(faddr->sin_addr.s_addr))
		{
			if(repeat_)
				addr->sin_addr.s_addr = inet_addr("1.0.0.1");
			else
				return -1;
		}
		else
		{
			if(addr_info.addr_type == IPv4_ADDRTYPE_HOST_PRIVATE
					|| addr_info.addr_type == IPv4_ADDRTYPE_NET_PRIVATE)
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
					default:
						LOG(ERROR) << _("Unknown private net type");
						return -2;
				};
			}
			else
			{
				addr->sin_addr.s_addr = htonl(ntohl(addr->sin_addr.s_addr) + 1);
			}
		}
		addr_info = GetIPv4Info(addr->sin_addr.s_addr);
	}
	return 0;
}

/**@beief Same as shoft_to_host, but shifts address to next first*/
int IPv4Generator::shift_to_next_host(struct sockaddr_in* addr, struct sockaddr_in* faddr)
{
	// TODO  if(mix_)
	if(ntohl(addr->sin_addr.s_addr) >= ntohl(faddr->sin_addr.s_addr))
	{
		if(repeat_)
			addr->sin_addr.s_addr = inet_addr("1.0.0.1");
		else
			return -1;
	}
	else
	{
		addr->sin_addr.s_addr = htonl(ntohl(addr->sin_addr.s_addr) + 1);
	}
	return shift_to_host(addr, faddr);
}

/**@brief produces ipv4 sequence (with port, if set)
 *
 * example:
 *   "12.168.1.1:25"
 *   "12.168.1.2:25"
 *    ...
 * Note that ip, that can't be set to a host in Internet would be skipped
 * (local, broadcast, reserved, ...).*/
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
		addr->sin_family = AF_INET;
		addr->sin_addr.s_addr = inet_addr("223.255.255.255");
	}
	else if(state[0] == 0x02)
	{ // regular
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
	{ // initial
		if(statemaxsz < sizeof(sockaddr_in) + 1 + sizeof(sockaddr_in))
 		{
			LOG(ERROR) << _("ipv4_generator: state buffer is too short.");
			*nextsz = 0;
			return *this;
		}
		char * state_cp = new char[*statesz];
		memset(state_cp, 0, *statesz);
		memcpy(state_cp, state, *statesz);
		size_t colon_pos = 0;
		size_t final_pos = 0;
		for(size_t pos = 0; pos < *statesz; ++pos)
		{
			if(colon_pos == 0 && state_cp[pos] == ':')
				colon_pos = pos;
			if(final_pos == 0 && state_cp[pos] == '-')
				final_pos = pos;
		}
		memset(state, 0, statemaxsz);
		state[0] = 0x02;
		addr = (sockaddr_in*)&state[1];
		addr->sin_family = AF_INET;
		faddr = (sockaddr_in*)&state[1 + sizeof(sockaddr_in)];
		faddr->sin_family = AF_INET;
		faddr->sin_addr.s_addr = inet_addr("223.255.255.255");
		if(final_pos == 0)
		{
			LOG(ERROR) << _("ipv4_generator: no final value set.");
		}
		else if(colon_pos >= final_pos)
		{
			LOG(ERROR) << _("ipv4_generator: wrong final and colon positions.");
			state_cp[final_pos] = 0;
		}
		else if(final_pos + 1 < *statesz)
		{
			if(inet_pton(AF_INET, &state_cp[final_pos + 1], &faddr->sin_addr) <= 0)
			{
				LOG(ERROR) << _("ipv4_generator: inet_pton with final error.")
					<< _("Message") << ": " << strerror(errno);
			}
			state_cp[final_pos] = 0;
		}
		if(colon_pos != 0)
		{
			addr->sin_port = htons((unsigned short)atoi(&state_cp[colon_pos+1]));
			state_cp[colon_pos] = 0;
		}
		else
		{
			addr->sin_port = 0;
		}
		if(inet_pton(AF_INET, state_cp, &addr->sin_addr) <= 0)
		{
			LOG(ERROR) << _("ipv4_generator: inet_pton error.") << " "
				<< _("Message") << ": " << strerror(errno);
			nextsz = 0;
			return *this;
		}
		delete [] state_cp;

		if(shift_to_host(addr, faddr) < 0)
		{
			state[0] = 0x03;
			*statesz = 1;
			return *this;
		}
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

