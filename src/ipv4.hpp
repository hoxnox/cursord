/**@author $username$ <$usermail$>
 * @date $date$ */

#ifndef __HX_IPV4_HPP__
#define __HX_IPV4_HPP__

#include <string>

namespace nx {

std::pair<uint32_t, uint32_t> CIDRtoIPv4Range(std::string str);

}

#endif // __HX_IPV4_HPP__
