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

/**@brief Generates Cartesian product from given value ranges.
 *
 * There are several generating strategies
 *
 * (1, 2, 3, 4) (A, B)
 *
 * * MIX_NONE {1A,1B,2A,2B,3A,3B,4A,4B}
 * * MIX_DIAGONAL (1A,2B,4A,1B,3A,4B,2A,3B)
 * * MIX_SHUFFLE (values are shuffled, depend on seed)
 *
 * TODO: optimize if we have random access iterators.
 *
 * ## MIX_DIAGONAL 
 *
 * Suppose we have n1 elements in first set, n2 in second, ... nk in k.
 * First thing we need to do - find the tuple (N1, ..., Nk) so that
 * N1 >= n1, N2 >= n2, ... Nk >=nk and gcd(Ni,Nj)=1 for all
 * i != j. After that we choose (a1, ..., ak) so a1 <= N1, ..., ak <= Nk
 * and generating all elements of that cyclic additive group. We skip
 * element for which exists aj so that aj >= nj.
 * */
template <class Iter>
class MixedCartesianProduct
{
public:
	enum MixMode
	{
		MIX_NONE,
		MIX_DIAGONAL,
		MIX_SHUFFLE,
	};
	typedef uint64_t SizeT;
	using ObjT=typename std::iterator_traits<Iter>::value_type;
	typedef std::pair<Iter, Iter> InRangeT;
	MixedCartesianProduct(std::vector<InRangeT> inputs,
	                      MixMode mix_mode = MIX_NONE,
	                      SizeT mix_seed = 0x98765);
	bool IsReady() const { return cnt_total_ != 0; };
	std::string StrError() const { return err_.str(); };

	std::string Get(std::string delim = ";") const;
	std::vector<ObjT> operator*() const;
	MixedCartesianProduct<Iter>& operator++();
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
			, mix_seed_(mix_seed)
			, is_cycle_(false)
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
		bool IsCycle() const { return is_cycle_; }
		std::unique_ptr<shufor::ShuforV> shufor_;
		std::vector<size_t> state_;
		std::vector<size_t> sizes_;
		const SizeT mix_seed_;
		bool is_cycle_;
	};
	std::unique_ptr<State> state_;
	void incCurrpos();
	std::vector<std::vector<ObjT> > data_;
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
	, mix_mode_(mix_mode)
	, cnt_total_(0)
	, cnt_done_(0)
{
	std::vector<size_t> sizes;
	for (auto input : inputs)
	{
		data_.emplace_back(input.first, input.second);
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
		is_cycle_ = true;
	}
	else
	{
		if (shufor_->IsCycle())
		{
			shufor_.reset(new shufor::ShuforV(sizes_, mix_seed_));
			is_cycle_ = true;
		}
		state_ = shufor_->GetNext();
	}
	return *this;
}

template<class Iter> std::vector<typename MixedCartesianProduct<Iter>::ObjT>
MixedCartesianProduct<Iter>::operator*() const
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

template<class Iter> inline MixedCartesianProduct<Iter>&
MixedCartesianProduct<Iter>::operator++()
{
	++(*state_);
	++cnt_done_;
	return *this;
}


template<class Iter> inline std::string
MixedCartesianProduct<Iter>::Get(std::string delim) const
{
	std::vector<ObjT> values = operator*();
	std::string separator = "";
	std::string result;
	for (const auto& val : values)
	{
		result += separator + std::string(val.begin(), val.end());
		separator = delim;
	}
	return result;
}

} //namespace

