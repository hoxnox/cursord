/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160318 13:43:31 */

#include <cstdint>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <memory>

using SizeT = uint64_t;

SizeT gcd(SizeT a, SizeT b);

bool is_coprime(SizeT val, const std::vector<SizeT>& vec);

/**@brief Simple prime criteria*/
bool is_prime_naive(SizeT n);

/**@brief Find the prime number greater or equal to the given*/
SizeT find_ge_prime(SizeT n);

/**@brief Find co-prime numbers, that is greater or equal to the given.
 *
 * This algorithm doesn't give the closest vector, but is very simple.*/
std::vector<SizeT> find_gt_coprime(const std::vector<SizeT>& vec);

class CycleGroup
{
public:
	using Element = std::vector<SizeT>;
	CycleGroup(const std::vector<SizeT>& sizes, SizeT seed = 0xabcdef);
	bool IsValid(const Element& el) const;
	class const_iterator
	{
	public:
		const_iterator& operator++();
		const Element& operator*() const { return curr_; }
		bool operator==(const const_iterator& rhv);
		bool operator!=(const const_iterator& rhv) { return !operator==(rhv); }
	private:
		const_iterator(const CycleGroup& cycle_group,
		               const Element& curr);
		Element curr_;
		SizeT curr_pos_;
		const CycleGroup* owner_;
	friend class CycleGroup;
	};
	const_iterator begin() const;
	const_iterator end() const;
private:
	std::vector<SizeT> seed_;
	const std::vector<SizeT> sizes_;
	const std::vector<SizeT> coprime_sizes_;
	SizeT power_;
friend class const_iterator;
};

namespace std {
	ostream& operator<<(ostream& os, const CycleGroup::Element& gr);
} // namespace

////////////////////////////////////////////////////////////////////////
// inline

inline bool
CycleGroup::const_iterator::operator==(const const_iterator& rhv)
{
	return owner_ == rhv.owner_ && curr_pos_ == rhv.curr_pos_;
}

inline CycleGroup::const_iterator
CycleGroup::begin() const
{
	const_iterator iter(*this, seed_);
	return iter;
}

inline CycleGroup::const_iterator
CycleGroup::end() const
{
	const_iterator iter(*this, seed_);
	iter.curr_pos_ = power_;
	return iter;
}

inline
CycleGroup::const_iterator::const_iterator(
	const CycleGroup& cycle_group,
	const Element& curr)
	: curr_(curr)
	, curr_pos_(0)
	, owner_(&cycle_group)
{
}

inline bool
CycleGroup::IsValid(const Element& el) const
{
	if (el.size() != sizes_.size())
		return false;
	for (auto i = 0; i < el.size(); ++i)
		if (el[i] >= sizes_[i])
			return false;
	return true;
}

