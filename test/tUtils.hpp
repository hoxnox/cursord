/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#include <Utils.hpp>
#include <fstream>
#include <cstdlib>
#include <string>
#include <unistd.h>

using namespace cursor;

class TestUtils : public ::testing::Test
{
protected:
	void SetUp() {}
	void TearDown() {}
};

TEST_F(TestUtils, read_file)
{
	char fname1[] = "cursortestXXXXXX";
	char fdata1[] = "one\ntwo\nthree";
	int fd1 = mkstemp(fname1);
	ASSERT_NE(-1, fd1);
	ASSERT_EQ(sizeof(fdata1)-1, write(fd1, fdata1, sizeof(fdata1)-1));
	close(fd1);

	char fname2[] = "cursortestXXXXXX";
	char fdata2[] = "four\nfive";
	int fd2 = mkstemp(fname2);
	ASSERT_NE(-1, fd2);
	ASSERT_EQ(sizeof(fdata2)-1, write(fd2, fdata2, sizeof(fdata2)-1));
	close(fd2);

	std::vector<Lines> lines = read_files({fname1, fname2});
	std::remove(fname1);
	std::remove(fname2);
	ASSERT_EQ(2, lines.size());
	ASSERT_EQ(3, lines[0].size());
	ASSERT_EQ(2, lines[1].size());
	ASSERT_EQ("one"  , lines[0][0]);
	ASSERT_EQ("two"  , lines[0][1]);
	ASSERT_EQ("three", lines[0][2]);
	ASSERT_EQ("four" , lines[1][0]);
	ASSERT_EQ("five" , lines[1][1]);
}

