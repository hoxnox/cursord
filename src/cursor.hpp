/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#ifndef __NX_CURSOR_HPP__
#define __NX_CURSOR_HPP__

#include <vector>
#include <string>
#include <deque>
#include <map>

#include <logging.h>
#include <gettext.h>
#include <memory>
#include <string.hpp>
#include "speedometer.hpp"
#include <shufor.h>

namespace sh=shufor;

namespace cursor {

class Cursor
{
public:
	struct Config
	{
	public:
		Config(const std::vector<std::string>&& urls,
		       const std::vector<std::string>&& extra_fnames = {},
		       const size_t                     shared_curr = 1,
		       const size_t                     shared_total = 1,
		       const bool                       extra_mix = false,
		       const std::string                extra_delim = ";")
			: urls(urls)
			, shared_curr(shared_curr)
			, shared_total(shared_total)
			, extra_fnames(extra_fnames)
			, extra_mix(extra_mix)
			, extra_delim(extra_delim)
		{
		}
		std::vector<std::string> urls;
		size_t                   shared_curr;
		size_t                   shared_total;
		std::vector<std::string> extra_fnames;
		bool                     extra_mix = false;
		std::string              extra_delim = ";";
	};
	typedef std::map<nx::String, nx::String> Args;
	Cursor();
	virtual ~Cursor();
	void Run(const Config&& cfg);
protected:
	int Next(const size_t count, std::deque<nx::String>& buf);
	virtual int do_next(const size_t count, std::deque<nx::String>& buf) = 0;
	nx::String initial_;
	bool isShared() const;
	size_t     shared_curr_;
	size_t     shared_total_;
private:
	enum
	{
		STATE_STOP  = 1,
		STATE_ERROR = 1 << 1
	};
	typedef std::vector<std::string> Lines;
	std::deque<nx::String>       buf_;
	size_t                       bufsz_;
	int                          state_;
	Speedometer                  speedometer_;
	std::vector<Lines>           extra_data_;
	std::string                  extra_delim_;
	bool                         extra_mix_;
	sh::TSize                    extra_totalsz_;
	std::unique_ptr<sh::ShuforV> shufor_;
	const sh::TSize SH_SEED = 0xdefedcbaUL;
};

inline bool
Cursor::isShared() const
{
	if (shared_total_ == 0 || shared_curr_ == 0 || shared_curr_ < shared_total_)
		return false;
	return true;
}

} // namespace

#endif /*__NX_CURSOR_HPP__*/

