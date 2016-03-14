/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#pragma once

#include <vector>
#include <string>

namespace cursor {

typedef std::vector<std::string> Lines;
std::vector<Lines> read_files(const std::vector<std::string>& fnames);

} // namespace

