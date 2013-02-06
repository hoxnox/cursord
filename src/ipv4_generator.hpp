/* @author $username$ <$usermail$>
 * @date $date$
 *
 * @brief IPv4 generator for cursor.*/

#ifndef __IPV4_GENERATOR__
#define __IPV4_GENERATOR__

#include <nx_socket.h>
#include <string>

namespace cursor {

class IPv4Generator
{
	public:
		IPv4Generator(const bool repeat,
		              const bool mix = false);
		int init(const char * init, const size_t initsz, char * state, const size_t &statesz);
		~IPv4Generator();

		IPv4Generator& operator()(char* state, size_t* statesz, size_t statemaxsz,
			char * next, size_t* nextsz, size_t nextmaxsz,
			int repeat);
	private:
		int shift_to_next_host(struct sockaddr_in* addr, struct sockaddr_in* faddr);
		int shift_bad_addr(struct sockaddr_in* addr, struct sockaddr_in* faddr);
		bool repeat_;
		bool mix_;

		uint32_t size_;
		uint32_t counter_;
		const uint32_t prime_number;
};

} // namespace

#endif // __IPV4_GENERATOR__

