/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <iterator>
#include <sstream>
#include <stdint.h>
#include <shufor.h>

namespace cursor {

template <class Iter>
class MixedCartesianProduct
{
public:
	enum MixMode
	{
		MIX_NONE,
		MIX_SHUFFLE
	};
	typedef uint64_t SizeT;
	using ObjT=typename std::iterator_traits<Iter>::value_type;
	typedef std::pair<Iter, Iter> InRangeT;
	MixedCartesianProduct(std::vector<InRangeT> inputs,
	                      MixMode mix_mode = MIX_NONE,
	                      SizeT mix_seed = 0x98765);
	bool IsReady() const { return cnt_total_ != 0; };
	std::string StrError() const { return err_.str(); };

	std::vector<ObjT> operator*();
	MixedCartesianProduct<Iter>& operator++() { ++(*state_); return *this; }
	operator bool() const { return cnt_total_ != cnt_done_; }

	/**@brief total elements in the CP*/
	SizeT GetCountTotal() const { return cnt_total_; }
	/**@brief count elements of the CP read*/
	SizeT GetCountDone() const { return cnt_done_; }
	/**@brfief count elements of the CP to be read*/
	SizeT GetCountLeft() const { return cnt_total_ - cnt_done_; }

	void RestoreByVal(std::vector<ObjT> value);
	void RestoreByCount(SizeT count);

	MixedCartesianProduct() = delete;
	MixedCartesianProduct(const MixedCartesianProduct&) = delete;
	MixedCartesianProduct& operator=(const MixedCartesianProduct&) = delete;

private:
	class State
	{
	public:
		State(const std::vector<size_t>& sizes,
		      MixMode mix_mode = MIX_NONE,
		      SizeT mix_seed = 0x98765)
			: sizes_(sizes)
			, state_(sizes.size(), 1)
			, shufor_(nullptr)
		{
			if (mix_mode == MIX_SHUFFLE)
			{
				shufor_.reset(new shufor::ShuforV(sizes, mix_seed));
				state_ = shufor_->GetNext();
			}
		}
		State() = delete;
		State(const State&) = delete;
		State& operator=(const State&) = delete;
		State& operator++();
		bool IsEnd() const
		{
			if (shufor_)
				return shufor_->IsCycle();
			for (auto i : state_)
				if (i != 0)
					return false;
			return true; 
		}
		std::unique_ptr<shufor::ShuforV> shufor_;
		std::vector<size_t> state_;
		std::vector<size_t> sizes_;
	};
	std::unique_ptr<State> state_;
	void incCurrpos();
	std::vector<std::vector<std::string> > data_;
	const SizeT mix_seed_;
	const MixMode mix_mode_;
	size_t cnt_total_;
	size_t cnt_done_;
	std::stringstream err_;
};

template<class Iter>
MixedCartesianProduct<Iter>::MixedCartesianProduct(
		std::vector<InRangeT> inputs,
		MixMode mix_mode,
		SizeT mix_seed)
	: state_(nullptr)
	, mix_seed_(mix_seed)
	, mix_mode_(mix_mode)
	, cnt_total_(0)
	, cnt_done_(0)
{
	std::vector<size_t> sizes;
	for (auto input : inputs)
	{
		data_.emplace_back(std::vector<std::string>(input.first, input.second));
		sizes.emplace_back(data_.back().size());
		if (data_.back().size() > 0)
			cnt_total_ = (cnt_total_ == 0 ? data_.back().size() : cnt_total_*data_.back().size());
	}
	state_.reset(new State(sizes, mix_mode, mix_seed));
}

template<class Iter> typename MixedCartesianProduct<Iter>::State&
MixedCartesianProduct<Iter>::State::operator++()
{
	if (shufor_ == nullptr)
	{
		for (ssize_t i = state_.size() - 1; i >= 0; --i)
		{
			if (state_[i] < sizes_[i])
			{
				++state_[i];
				return *this;
			}
			state_[i] = 1;
		}
	}
	else
	{
		state_ = shufor_->GetNext();
	}
	return *this;
}

template<class Iter> std::vector<typename MixedCartesianProduct<Iter>::ObjT>
MixedCartesianProduct<Iter>::operator*()
{
	if (!state_)
		return std::vector<typename MixedCartesianProduct<Iter>::ObjT>();
	std::vector<ObjT> rs;
	for (size_t i = 0; i < state_->state_.size(); ++i)
		rs.emplace_back(data_[i][state_->state_[i] - 1]);
	return rs;
}

template<class Iter> void
MixedCartesianProduct<Iter>::RestoreByVal(
		std::vector<MixedCartesianProduct<Iter>::ObjT> value)
{
}

template<class Iter> void
MixedCartesianProduct<Iter>::RestoreByCount(SizeT count)
{
}

} //namespace

