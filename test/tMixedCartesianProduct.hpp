/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#include <MixedCartesianProduct.hpp>
#include <string>
#include <vector>

using namespace cursor;

class TestMixedCartesianProduct : public ::testing::Test
{
protected:
	void SetUp()
	{
		data1.push_back("one");
		data1.push_back("two");
		data1.push_back("three");

		data2.push_back("four");
		data2.push_back("five");

		data3.push_back("six");
		data3.push_back("seven");
		data3.push_back("eight");

		etalon = std::vector<std::vector<std::string> >(
		{
			{"one", "four", "six"},
			{"one", "four", "seven"},
			{"one", "four", "eight"},
			{"one", "five", "six"},
			{"one", "five", "seven"},
			{"one", "five", "eight"},
			{"two", "four", "six"},
			{"two", "four", "seven"},
			{"two", "four", "eight"},
			{"two", "five", "six"},
			{"two", "five", "seven"},
			{"two", "five", "eight"},
			{"three", "four", "six"},
			{"three", "four", "seven"},
			{"three", "four", "eight"},
			{"three", "five", "six"},
			{"three", "five", "seven"},
			{"three", "five", "eight"}
		});
	}
	void TearDown()
	{
	}
	std::vector<std::string> data1;
	std::vector<std::string> data2;
	std::vector<std::string> data3;
	std::vector<std::vector<std::string> > etalon;
};

TEST_F(TestMixedCartesianProduct, mix_none)
{
	MixedCartesianProduct<std::vector<std::string>::const_iterator> mixer({
				make_pair(data1.begin(), data1.end()),
				make_pair(data2.begin(), data2.end()),
				make_pair(data3.begin(), data3.end())
			});
	ASSERT_EQ(3*2*3, mixer.GetCountTotal());
	ASSERT_EQ(0, mixer.GetCountDone());
	ASSERT_EQ(3*2*3, mixer.GetCountLeft());
	ASSERT_TRUE(mixer.IsReady()) << mixer.GetCountTotal();
	for (size_t i = 0; i < 3*2*3 && (bool)mixer; ++i, ++mixer)
	{
		std::vector<std::string> result = *mixer;
		ASSERT_EQ(etalon[i].size(), result.size());
		EXPECT_EQ(etalon[i], result) << " i = " << i;
	}
}

TEST_F(TestMixedCartesianProduct, mix_shuffle_1)
{
	MixedCartesianProduct<std::vector<std::string>::const_iterator> mixer({
				make_pair(data1.begin(), data1.end()),
				make_pair(data2.begin(), data2.end()),
				make_pair(data3.begin(), data3.end())
			}, MixedCartesianProduct<std::vector<std::string>::const_iterator>::MIX_SHUFFLE, 1);
	EXPECT_EQ(std::vector<std::string>({"three", "five", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "five", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "four", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "five", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "five", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "five", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "four", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "four", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "five", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "four", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "four", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "four", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "five", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "four", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "four", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "five", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "five", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "four", "six"  }), *mixer);
}

TEST_F(TestMixedCartesianProduct, mix_shuffle_abcdef0e)
{
	MixedCartesianProduct<std::vector<std::string>::const_iterator> mixer({
				make_pair(data1.begin(), data1.end()),
				make_pair(data2.begin(), data2.end()),
				make_pair(data3.begin(), data3.end())
			}, MixedCartesianProduct<std::vector<std::string>::const_iterator>::MIX_SHUFFLE, 0xabcdef0e);
	EXPECT_EQ(std::vector<std::string>({"three", "four", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "five", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "five", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "four", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "five", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "five", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "four", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "five", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "five", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "five", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "four", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "four", "seven"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "five", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "four", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"one"  , "four", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "four", "eight"}), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"three", "five", "six"  }), *mixer); ++mixer;
	EXPECT_EQ(std::vector<std::string>({"two"  , "four", "seven"}), *mixer);
}

