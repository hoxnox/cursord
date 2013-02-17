#include <shuffle.hpp>

namespace cursor {

/**@class ShuffleGenerator
 * @brief Random generator [0, size]
 *
 * Used to pass through every number from 0 to size in random order.
 * Internally we use Linear Feedback Shift Register (LFSR) in Galua
 * configuration with prime polynomial gave full cycle.*/

const unsigned char ShuffleGenerator::prime_poly[32] = {
	  1, // 00000001
	  3, // 00000011
	  3, // 00000011
	  3, // 00000011
	  5, // 00000101
	  3, // 00000011
	  3, // 00000011
	 29, // 00011101
	 17, // 00010001
	  9, // 00001001
	  5, // 00000101
	 83, // 01010011
	 27, // 00011011
	 43, // 00101011
	  3, // 00000011
	 45, // 00101101
	  9, // 00001001
	129, // 10000001
	 39, // 00100111
	  9, // 00001001
	  5, // 00000101
	  3, // 00000011
	 33, // 00100001
	 27, // 00011011
	  9, // 00001001
	 71, // 01000111
	 39, // 00100111
	  9, // 00001001
	  5, // 00000101
	 83, // 01010011
	  9, // 00001001
	197  // 11000101
};

ShuffleGenerator::ShuffleGenerator(const uint32_t seed  /*= 0xabcdefed*/)
	: pow2_approx_(0)
	, size_(0)
	, count_(0)
	, seed_(seed)
	, register_(1)
	, initial_(1)
	, is_cycle_(false)
{
}

inline uint32_t crop(const uint32_t num, unsigned char pow2)
{
	if(pow2 >= 32)
		return num;
	uint32_t mask = 0xffffffff;
	mask <<= pow2;
	mask = ~mask;
	return num & mask;
}

void ShuffleGenerator::init_register()
{
	register_ = seed_ == 0 ? 1 : seed_;
	register_ = crop(register_, pow2_approx_ + 1);
	if(register_ > size_)
		GetNext();
	initial_ = register_;
}


/*@brief Set size*/
void ShuffleGenerator::Init(uint32_t size)
{
	is_cycle_ = false;
	if(size == 0)
		return;
	size_ = size;
	pow2_approx_ = 0;
	if(size > 2147483647L)
	{
		pow2_approx_ = 31;
	}
	else
	{
		while(pow2_approx_ < 32)
		{
			if(size >= (1 << (pow2_approx_ + 1)))
				++pow2_approx_;
			else
				break;
		}
	}
	init_register();
}

/*@brief Get next number*/
uint32_t ShuffleGenerator::GetNext()
{
	uint32_t result = register_;
	do{
		if(register_ & 0x00000001)
			register_ = ((register_ ^ prime_poly[pow2_approx_ ]) >> 1) | (1 << pow2_approx_);
		else
			register_ >>= 1;
	}while(register_ > size_);
	if(register_ == initial_)
		is_cycle_ = true;
	return result;
}

/*@brief Restore ShuffleGenerator to previous state, using last
 * generated value*/
void ShuffleGenerator::RestoreVal(const uint32_t size, const uint32_t val)
{
	Init(size);
	register_ = val;
}

/*@brief Restore ShuffleGenerator to previous state, usend count
 * of number generated*/
void ShuffleGenerator::RestoreCnt(const uint32_t size, const uint32_t count)
{
	if(size < count)
		return;
	Init(size);
	for(uint32_t i = 0; i < count; ++i)
		GetNext();
}

} // namespace 

