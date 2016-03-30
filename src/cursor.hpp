/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#ifndef __NX_CURSOR_HPP__
#define __NX_CURSOR_HPP__

#include <vector>
#include <string>
#include <deque>
#include <map>
#include <fstream>

#include <logging.h>
#include <gettext.h>
#include <memory>
#include <string.hpp>
#include "speedometer.hpp"
#include <shufor.h>
#include <MixedCartesianProduct.hpp>

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
			, extra_mix(extra_mix)
			, extra_delim(extra_delim)
			, extra_fnames(extra_fnames)
		{
		}
		std::vector<std::string> urls;
		size_t                   shared_curr;
		size_t                   shared_total;
		bool                     extra_mix;
		std::string              extra_delim;
		std::vector<std::string> extra_fnames;
	};
	typedef std::map<nx::String, nx::String> Args;
	Cursor();
	virtual ~Cursor();
	void Run(const Config&& cfg);
protected:
	int Next(const size_t count, std::deque<nx::String>& buf);
	/**@brief Reinitialize cursor. After that call do_next must produce
	 * the same element as it was initialy started.*/
	//virtual int reset(const Config&& cfg) = 0;
	/**@brief Generate next count elements and push_back em into buf,
	 * return negative on error.*/
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
	class Line : public std::string
	{
	public:
		Line() {}
		template<class InIter>
		Line(InIter begin, InIter end) : std::string(begin, end) {}
		friend std::istream & operator>>(std::istream & is, Line & line)
		{
			return std::getline(is, line);
		}
	};
	typedef std::istream_iterator<Line> LineIterT;
	typedef MixedCartesianProduct<LineIterT> MixerT ;
	typedef std::vector<std::string> Lines;

	std::string                  extra_delim_;
	std::unique_ptr<MixerT>      extra_state_;
	std::deque<nx::String>       buf_;
	size_t                       bufsz_;
	int                          state_;
	Speedometer                  speedometer_;
	const sh::TSize SH_SEED = 0xdefedcbaUL;
};

inline bool
Cursor::isShared() const
{
	if (shared_total_ == 0 || shared_curr_ == 0 || shared_total_ < shared_curr_)
		return false;
	return true;
}

} // namespace

#endif /*__NX_CURSOR_HPP__*/

