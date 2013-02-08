/* @author $username$ <$usermail$>
 * @date $date$ */

#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

#include <stdint.h>

namespace cursor {

class ShuffleGenerator
{
	public:
		ShuffleGenerator(const uint32_t seed  = 0xabcdefed);
		void Init(uint32_t size);

		uint32_t GetNext();

		void RestoreVal(const uint32_t size, const uint32_t val);
		void RestoreCnt(const uint32_t size, const uint32_t count);
		bool IsCycle() const { return is_cycle_; }

	private:
		void init_register();
		static const unsigned char prime_poly[32];
		unsigned char pow2_approx_;
		uint32_t size_;
		uint32_t count_;
		uint32_t register_;
		uint32_t seed_;
		uint32_t initial_;
		uint32_t crop_mask_;
		bool is_cycle_;
};

} // namespace

#endif // __RANDOM_HPP__
