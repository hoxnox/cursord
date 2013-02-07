#include <shuffle.hpp>

namespace cursor {

/**@class ShuffleGenerator
 * @brief Random generator [0, size]
 *
 * Used to pass through every number from 0 to size in random order.*/

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

// Linear Feedback Shift Register (LFSR) used with prime polynomial gives full cycle,
// so to generate 2^N random shuffle we can use LFSR.

ShuffleGenerator::ShuffleGenerator(const uint32_t seed  /*= 0xabcdefed*/)
	: pow2_approx_(0)
	, size_(0)
	, count_(0)
	, register_(seed)
	, initial_(0)
{
}

inline uint32_t crop(const uint32_t num, unsigned char pow2)
{
	if(pow2 >= 32)
		return num;
	uint32_t mask = 0xffffffff;
	return num & (~(mask << pow2));
}

/*@brief Set size*/
void ShuffleGenerator::Init(uint32_t size)
{
	if(size == 0)
		return;
	size_ = size;
	pow2_approx_ = 1;
	while(pow2_approx_ < 32)
	{
		if(size > (1 << pow2_approx_))
			++pow2_approx_;
		else
			break;
	}
	register_ = crop(register_, pow2_approx_);
	if(register_ > size)
		GetNext();
}

/*@brief Get next number*/
uint32_t ShuffleGenerator::GetNext()
{
	uint32_t result = register_;
	while(register_ > size_)
	{
		if(register_ & 0x00000001)
			register_ = ((register_ ^ prime_poly[pow2_approx_]) >> 1) | (1 << pow2_approx_);
		else
			register_ & (~(0xffffffff << (pow2_approx_ - 1)));
	}
	return result;
}

/*@brief Restore ShuffleGenerator to previous state, using last
 * generated value*/
void ShuffleGenerator::RestoreVal(const uint32_t size, const uint32_t val)
{
}

/*@brief Restore ShuffleGenerator to previous state, usend count
 * of number generated*/
void ShuffleGenerator::RestoreCnt(const uint32_t size, const uint32_t count)
{
}

/*@brief Tells how much numbers were passed*/
float ShuffleGenerator::Progress()
{
	return 0;
}

}

