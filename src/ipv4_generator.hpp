/* @author $username$ <$usermail$>
 * @date $date$
 * 
 * @brief IPv4 generator for cursor.*/

#ifndef __IPV4_GENERATOR__
#define __IPV4_GENERATOR__

#include <nx_socket.h>

namespace cursor {

class IPv4Generator
{
	public:
		IPv4Generator(const char * init,
		              const size_t initsz,
		              const bool repeat,
		              const bool mix = false);
		~IPv4Generator();

		IPv4Generator& operator()(char* state, size_t* statesz, size_t statemaxsz,
			char * next, size_t* nextsz, size_t nextmaxsz,
			int repeat);
	private:
		int shift_to_next_host(struct sockaddr_in* addr, struct sockaddr_in* faddr);
		int shift_to_host(struct sockaddr_in* addr, struct sockaddr_in* faddr);
		bool repeat_;
		bool mix_;
};

} // namespace

#endif // __IPV4_GENERATOR__

