/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include <shuffle.hpp>

class TShuffle : public testing::Test
{
	protected:
		void SetUp()
		{
			ASSERT_NO_THROW(sg = new cursor::ShuffleGenerator(0xabcdefed));
		}
		void TearDown()
		{
		}
		static char result_9[9];
		static char result_100[100];
		cursor::ShuffleGenerator * sg;

};

char TShuffle::result_9[9] = {
	7, 5, 6, 3, 8, 4, 2, 1, 9
};

char TShuffle::result_100[100] = { 
	61,  95,  55,  90,  45,  87,  53,  91,  54,  27,  76,  38,  19,  72,
	36,  18,   9,  69,  99,  56,  28,  14,   7,  66,  33,  81,  62,  31,
	78,  39,  82,  41,  85,  58,  29,  79,  51,  88,  44,  22,  11,  68,
	34,  17,  73,  60,  30,  15,  70,  35,  80,  40,  20,  10,   5,  67,
	96,  48,  24,  12,   6,   3,  64,  32,  16,   8,   4,   2,   1,  65,
	97,  63,  94,  47,  86,  43,  84,  42,  21,  75, 100,  50,  25,  77,
	57,  93,  59,  92,  46,  23,  74,  37,  83,  52,  26,  13,  71,  98,
	49,  89
};

TEST_F(TShuffle, generate)
{
	sg->Init(9);
	for(size_t i = 0; i < 9; ++i)
		EXPECT_EQ(sg->GetNext(), result_9[i]);
	std::cout << std::endl;
	sg->Init(100);
	for(size_t i = 0; i < 100; ++i)
		EXPECT_EQ(sg->GetNext(), result_100[i]);
	std::cout << std::endl;
}

TEST_F(TShuffle, restore)
{
	sg->RestoreVal(100, 11);
	for(size_t i = 40; i < 100; ++i)
		EXPECT_EQ(sg->GetNext(), result_100[i]);
	sg->RestoreCnt(100, 40);
	for(size_t i = 40; i < 100; ++i)
		EXPECT_EQ(sg->GetNext(), result_100[i]);
}


