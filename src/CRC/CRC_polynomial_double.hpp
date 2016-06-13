#ifndef CRC_POLYNOMIAL_DOUBLE_HPP_
#define CRC_POLYNOMIAL_DOUBLE_HPP_

#include <map>

#include "CRC_polynomial.hpp"

template <typename B>
class CRC_polynomial_double : public CRC_polynomial<B>
{
private:
	const int cut_index;

public:
	CRC_polynomial_double(const int K, std::string poly_key, const int cut_index, const int n_frames = 1);
	virtual ~CRC_polynomial_double(){};

	virtual void build(mipp::vector<B>& U_K);
};

#endif /* CRC_POLYNOMIAL_DOUBLE_HPP_ */