/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include <shuffle.hpp>

class TShuffle : public testing::Test
{
	protected:
		void SetUp()
		{
		}
		void TearDown()
		{
		}
};

TEST_F(TShuffle, main)
{
	cursor::ShuffleGenerator sg;
	sg.Init(9);
	for(size_t i = 0; i < 9; ++i)
		std::cout << sg.GetNext() << " " << std::endl;
}

