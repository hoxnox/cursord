/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160318 13:43:31 */

#include <cstdlib>
#include <Math.hpp>

CycleGroup::CycleGroup(const std::vector<SizeT>& sizes, SizeT seed)
	: sizes_(sizes)
	, coprime_sizes_(find_gt_coprime(sizes))
	, power_(0)
{
	seed_.reserve(coprime_sizes_.size());
	std::srand((int)seed);
	for (const auto& i : sizes_)
		seed_.emplace_back(((SizeT)std::rand())%i);
	for (const auto& i : coprime_sizes_)
		power_ = (power_ == 0 ? 1 : power_) * i;
}

CycleGroup::const_iterator&
CycleGroup::const_iterator::operator++()
{
	if (curr_pos_ >= owner_->power_)
		return *this;
	while (curr_pos_ < owner_->power_)
	{
		for (ssize_t i = owner_->coprime_sizes_.size() - 1; i >= 0; --i)
		{
			if (curr_[i] < owner_->coprime_sizes_[i] - 1)
				++curr_[i];
			else
				curr_[i] = 0;
		}
		++curr_pos_;
		if (owner_->IsValid(curr_))
			break;
	}
	return *this;
}

SizeT
gcd(SizeT a, SizeT b)
{
	if(!b)
		return a;
	return gcd(b, a % b);
}

bool
is_coprime(SizeT val, const std::vector<SizeT>& vec)
{
	SizeT mt = 1;
	for (const auto& i : vec)
		mt *= i;
	if (gcd(val, mt) == 1)
		return true;
	return false;
}

bool
is_prime_naive(SizeT n)
{
	int i,root;
	if (n < 4 && n > 0)
		return true;
	if (n%2 == 0 || n%3 == 0)
		return false;
	root = (SizeT)sqrt(n);
	for (i=5; i<=root; i+=6)
	{
		if (n%i == 0)
			return false;
	}
	for (i=7; i<=root; i+=6)
	{
		if (n%i == 0)
			return false;
	}
	return true;
}

SizeT
find_ge_prime(SizeT n)
{
	SizeT i = n;
	while (1)
	{
		if (is_prime_naive(i))
			break;
		i++;
	}
	return i;
}

std::vector<SizeT>
find_gt_coprime(const std::vector<SizeT>& vec)
{
	if (vec.empty())
		return std::vector<SizeT>();
	std::vector<std::pair<SizeT, SizeT>> svec;
	for (SizeT i = 0; i < vec.size(); ++i)
		svec.emplace_back(std::make_pair(i, vec[i]));
	std::sort(svec.begin(), svec.end(),
			[](std::pair<SizeT, SizeT> a, std::pair<SizeT, SizeT> b)
			{ return a.second < b.second; });
	std::vector<SizeT> result(vec.size());
	result[svec[0].first] = svec[0].second;
	SizeT prev = svec[0].second;
	for (SizeT i = 1; i < svec.size(); ++i)
	{
		SizeT curr = find_ge_prime(svec[i].second);
		if (prev > 0 && curr <= prev)
			curr = find_ge_prime(prev + 1);
		result[svec[i].first] = curr;
		prev = curr;
	}
	return result;
}

namespace std {

ostream& operator<<(ostream& os, const CycleGroup::Element& el)
{
	os << "{";
	std::string delim;
	for (const auto& i : el)
	{
		os << delim << i;
		delim = ",";
	}
	os << "}";
	return os;
}

} // namespace 

