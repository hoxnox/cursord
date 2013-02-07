/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$
 * @brief Test launcher.*/

// Google Testing Framework
#include <gtest/gtest.h>

// test cases
//#include "tgenerators.hpp"
#include "tshuffle.hpp"

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


