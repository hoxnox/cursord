/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#include <Math.hpp>
#include <algorithm>

class TestMath : public ::testing::Test
{
protected:
	void SetUp() override
	{}
	void TearDown() override
	{}
};

TEST_F(TestMath, gcd)
{
	EXPECT_EQ(1, gcd(1, 2));
	EXPECT_EQ(2, gcd(2, 2));
	EXPECT_EQ(1, gcd(3, 2));
	EXPECT_EQ(3, gcd(3, 3));
	EXPECT_EQ(6, gcd(12, 18));
}

TEST_F(TestMath, is_coprime)
{
	EXPECT_TRUE(is_coprime(1, {2}));
	EXPECT_TRUE(is_coprime(1, {2, 3, 4}));
	EXPECT_TRUE(is_coprime(4, {3, 5, 7, 9}));
	EXPECT_FALSE(is_coprime(2, {1, 3, 5, 7, 9, 11, 4}));
	EXPECT_FALSE(is_coprime(12, {18}));
}

TEST_F(TestMath, is_prime_naive)
{
	EXPECT_TRUE (is_prime_naive(2));
	EXPECT_TRUE (is_prime_naive(3));
	EXPECT_FALSE(is_prime_naive(4));
	EXPECT_TRUE (is_prime_naive(5));
	EXPECT_FALSE(is_prime_naive(6));
	EXPECT_TRUE (is_prime_naive(7));
	EXPECT_FALSE(is_prime_naive(8));
	EXPECT_FALSE(is_prime_naive(9));
	EXPECT_FALSE(is_prime_naive(10));
	EXPECT_TRUE (is_prime_naive(11));
	EXPECT_FALSE(is_prime_naive(12));
}

TEST_F(TestMath, find_ge_prime)
{
	EXPECT_EQ(1, find_ge_prime(0));
	EXPECT_EQ(2, find_ge_prime(2));
	EXPECT_EQ(3, find_ge_prime(3));
	EXPECT_EQ(5, find_ge_prime(4));
	EXPECT_EQ(5, find_ge_prime(5));
	EXPECT_EQ(7, find_ge_prime(6));
	EXPECT_EQ(7, find_ge_prime(7));
	EXPECT_EQ(11,find_ge_prime(8));
	EXPECT_EQ(18446744073709551419UL,find_ge_prime(18446744073709551416UL));
}

TEST_F(TestMath, find_gt_coprime_v)
{
	EXPECT_EQ(std::vector<SizeT>({2, 11, 3, 17, 5, 7, 13}),
			find_gt_coprime({2, 6, 3, 8, 4, 5, 7}));
	EXPECT_EQ(std::vector<SizeT>({4, 11, 23, 17, 5, 7, 13}),
			find_gt_coprime({4, 6, 22, 8, 4, 5, 7}));
}

std::string unindent(const char* p)
{
	std::string result;
	if (p[0] == '\n') ++p;
	const char* p_leading = p;
	while (std::isspace(*p) && *p != '\n')
		++p;
	size_t leading_len = p - p_leading;
	while (*p)
	{
		result += *p;
		if (*p == '\n')
		{
			++p;
			for (size_t i = 0; i < leading_len; ++i)
				if (p[i] != p_leading[i])
					goto dont_skip_leading;
			p += leading_len;
		}
		else
			++p;
	  dont_skip_leading: ;
	}
	return result;
}

TEST_F(TestMath, CycleGroup)
{
	CycleGroup gr({4, 6, 5}, 0x1234);
	std::stringstream ss;
	std::for_each(gr.begin(), gr.end(),
		[&ss](const CycleGroup::Element& e) { ss << e << ';'; });
	const std::string etalon = 
		"{2,0,2};{3,1,3};{0,2,4};{1,3,0};{2,4,1};{3,5,2};{1,0,4};"
		"{2,1,0};{3,2,1};{0,3,2};{1,4,3};{2,5,4};{0,0,1};{1,1,2};"
		"{2,2,3};{3,3,4};{0,4,0};{1,5,1};{3,0,3};{0,1,4};{1,2,0};"
		"{2,3,1};{3,4,2};{0,5,3};{2,0,0};{3,1,1};{0,2,2};{1,3,3};"
		"{2,4,4};{3,5,0};{1,0,2};{2,1,3};{3,2,4};{0,3,0};{1,4,1};"
		"{2,5,2};{0,0,4};{1,1,0};{2,2,1};{3,3,2};{0,4,3};{1,5,4};"
		"{3,0,1};{0,1,2};{1,2,3};{2,3,4};{3,4,0};{0,5,1};{2,0,3};"
		"{3,1,4};{0,2,0};{1,3,1};{2,4,2};{3,5,3};{1,0,0};{2,1,1};"
		"{3,2,2};{0,3,3};{1,4,4};{2,5,0};{0,0,2};{1,1,3};{2,2,4};"
		"{3,3,0};{0,4,1};{1,5,2};{3,0,4};{0,1,0};{1,2,1};{2,3,2};"
		"{3,4,3};{0,5,4};{2,0,1};{3,1,2};{0,2,3};{1,3,4};{2,4,0};"
		"{3,5,1};{1,0,3};{2,1,4};{3,2,0};{0,3,1};{1,4,2};{2,5,3};"
		"{0,0,0};{1,1,1};{2,2,2};{3,3,3};{0,4,4};{1,5,0};{3,0,2};"
		"{0,1,3};{1,2,4};{2,3,0};{3,4,1};{0,5,2};{2,0,4};{3,1,0};"
		"{0,2,1};{1,3,2};{2,4,3};{3,5,4};{1,0,1};{2,1,2};{3,2,3};"
		"{0,3,4};{1,4,0};{2,5,1};{0,0,3};{1,1,4};{2,2,0};{3,3,1};"
		"{0,4,2};{1,5,3};{3,0,0};{0,1,1};{1,2,2};{2,3,3};{3,4,4};"
		"{0,5,0};";
	EXPECT_EQ(etalon, ss.str());
}

