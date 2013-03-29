/* @author $username$ <$usermail$>
 * @date $date$
 *
 * @brief IPv4 generator for cursor.*/

#ifndef __IPV4_GENERATOR__
#define __IPV4_GENERATOR__

#include <nx_socket.h>
#include <shuffle.hpp>
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
		void SetSkipPrivate(bool skip) { skip_private_ = skip; }
		size_t size() const { return size_+1; }
		size_t pos() const { return counter_; }
	private:
		int next              (uint32_t &curr, const uint32_t final);
		int shift_to_next_host(struct sockaddr_in* addr, const struct sockaddr_in* faddr);
		int shift_bad_addr    (struct sockaddr_in* addr, const struct sockaddr_in* faddr);
		bool repeat_;
		bool mix_;

		size_t           size_;
		uint32_t         size_approx_;
		size_t           counter_;
		const uint32_t   prime_number_;
		uint32_t         initial_;
		ShuffleGenerator shuffle_;
		bool             skip_private_;
};

} // namespace

#endif // __IPV4_GENERATOR__

