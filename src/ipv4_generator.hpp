/* @author hoxnox <hoxnox@gmail.com>
 * @date 20160301 16:00:58
 *
 * @brief IPv4 generator for cursor.*/

#ifndef __IPV4_GENERATOR__
#define __IPV4_GENERATOR__

#include <shufor.h>
#include <string>
#include <memory>

namespace cursor {

using namespace shufor;

class IPv4Generator
{
	public:
		IPv4Generator(const bool repeat,
		              const bool mix = false);
		int init(const char * init, const size_t initsz,
		         char * state, const size_t &statesz,
			 const uint32_t shuffle_restore_val = 0);
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
		bool initialized() { return size_ > 0; }
		bool repeat_;
		bool mix_;

		size_t           size_;
		uint32_t         size_approx_;
		size_t           counter_;
		const uint32_t   prime_number_;
		uint32_t         initial_;
		std::shared_ptr<Shufor> shuffle_;
		bool             skip_private_;
};

} // namespace

#endif // __IPV4_GENERATOR__

