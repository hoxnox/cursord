/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#include "Utils.hpp"
#ifndef LOG
#include <SimpleLogger.hpp>
SIMPLE_LOGGER_INIT
#define LOG(INFO) ILOG
#endif

#include <fstream>
#ifndef _
#define _(x) (x)
#endif

namespace cursor {

std::vector<Lines>
read_files(const std::vector<std::string>& fnames)
{
	if (fnames.empty())
		return std::move(std::vector<Lines>());
	std::vector<Lines> rs;
	for (auto fname : fnames)
	{
		Lines lines;
		std::ifstream ifile(fname);
		if (!ifile.is_open())
		{
			LOG(INFO) << _("Error opening extra file.")
			          << _(" Filename: ") << fname;
			return std::move(std::vector<Lines>());
		}
		for (std::string line; std::getline(ifile, line);)
			lines.push_back(line);
		rs.push_back(lines);
	}
	return rs;
}

} // namespace

