/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#include "cursor_generator.hpp"
#include "ipv4_generator.hpp"
#include <limits>
#include <cstring>
#include <typeinfo>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace cursor {

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

///////////////////////////////////////////////////////////////////////////
// CursorGenerator

inline long max(const long lhv, const long rhv)
{
	return lhv > rhv ? lhv : rhv;
}

void CursorGenerator::init(const Cursor::Args args)
{
	bool mix = false, priv = false;
	uint32_t restore = 0;
	state_   = (char *)malloc(statemaxsz_);
	memset(state_, 0, statemaxsz_);
	nextbuf_ = (char *)malloc(nextbufmaxsz_);
	memset(nextbuf_, 0, nextbufmaxsz_);
	for(Cursor::Args::const_iterator arg = args.begin(); arg != args.end(); ++arg)
	{
		nx::String tmp = arg->second;
		tmp.trim();
		if(arg->first == "repeat")
		{
			if(tmp.toLower() != L"false" && tmp.toLower() != L"0")
				repeat_ = 1;
		}
		else if(arg->first == "private")
		{
			if(tmp.toLower() != L"false" && tmp.toLower() != L"0")
				priv = true;
		}
		else if(arg->first == "init")
		{
			std::string init_utf8 = arg->second.toUTF8();
			if(init_utf8.length()  > statemaxsz_)
			{
				statemaxsz_ = init_utf8.length() + 1;
				state_ = (char*)realloc(state_, statemaxsz_);
				memset(state_, 0, statemaxsz_);
			}
			memcpy(state_, init_utf8.data(), init_utf8.length());
			statesz_ = init_utf8.length();
		}
		else if(arg->first == "mix")
		{
			if(tmp.toLower() != L"false" && tmp.toLower() != L"0")
				mix = true;
		}
		else if(arg->first == "restore")
		{
			restore = ntohl(inet_addr(arg->second.toASCII().c_str()));
			mix = true;
		}
		else if(arg->first == "name")
		{
			name_ = tmp.toLower();
		}
		else if(arg->first == "suffix")
		{
			suffix_ = arg->second;
		}
		else if(arg->first == "prefix")
		{
			prefix_ = arg->second;
		}
		else
		{
			LOG(WARNING) << ("Unsupported argument.") << " "
				<< arg->first << " = " << arg->second;
		}
	}
	if(name_.empty())
	{
		LOG(ERROR) << _("Generator name is not set.");
		return;
	}

	if(priv && name_ != L"ipv4")
		LOG(WARNING) << ("Unsupported argument \"private\" for this generator type.");

	if(name_ == L"int")
	{
		generator = simple_uint_generator;
	}
	else if(name_ == L"ipv4")
	{
		//IPv4Generator * gipv4 = new IPv4Generator(repeat_, mix);
                IPv4Generator gipv4(repeat_, mix);
		if(priv)
			gipv4.SetSkipPrivate(false);
		statesz_ = gipv4.init(state_, statesz_, state_, statemaxsz_, restore);
		if (isShared() && restore != 0)
			do_next_fake_count_ = shared_curr_ - 1;
		generator = gipv4;
	}
	else
	{
		LOG(ERROR) << _("Unsupported generator.") << " "
			<< name_.toUTF8();
	}
}

CursorGenerator::CursorGenerator(const Cursor::Args args)
	: Cursor()
	, do_next_fake_count_(0)
	, nextbufsz_(0)
	, nextbufmaxsz_(2048)
	, statesz_(0)
	, statemaxsz_(2048)
	, generator(NULL)
	, repeat_(0)
{
	init(args);
}

CursorGenerator::~CursorGenerator()
{
	if(state_)
		free(state_);
	if(nextbuf_)
		free(nextbuf_);
}

int CursorGenerator::do_next(const size_t count, std::deque<nx::String>& buf)
{
	if(generator == NULL)
		return 0;
	size_t i = 0;
	for(; i < count; ++i)
	{
		if (do_next_fake_count_ == 0)
		{
			generator(state_, &statesz_, statemaxsz_,
			          nextbuf_, &nextbufsz_, nextbufmaxsz_,
			          repeat_);
			if(nextbufsz_ == 0)
				break;
			buf.push_back(prefix_ + nx::String(nextbuf_, nextbuf_ + nextbufsz_) + suffix_);
		}
		else
		{
			buf.push_back(nx::String::fromASCII("FAKE"));
			--do_next_fake_count_;
		}
	}
	if(name_ == "ipv4" && !repeat_)
	{
		IPv4Generator * gen = generator.target<IPv4Generator>();
		if(gen != NULL)
		{
			size_t sz = gen->size();
			size_t ps = gen->pos();
			int percent = ((float)ps / sz)*100;
			LOG(INFO) << _("Progress") << ": " << ps << "/" << sz
				<< " (" << percent << "%)";
		}
	}
	return 0;
}

} // namespace

