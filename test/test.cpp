/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160301 16:00:58
 * @copyright hoxnox
 * @brief Test launcher.*/

// Google Testing Framework
#include <gtest/gtest.h>

// test cases
//#include "tgenerators.hpp"
#include "tipv4.hpp"
#include "tUtils.hpp"
#include "tMixedCartesianProduct.hpp"

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

