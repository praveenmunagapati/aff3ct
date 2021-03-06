#include <vector>
#include <cmath>
#include <sstream>

#include "Tools/Exception/exception.hpp"

#include "Encoder_polar.hpp"

using namespace aff3ct::module;

template <typename B>
Encoder_polar<B>
::Encoder_polar(const int& K, const int& N, const std::vector<bool>& frozen_bits, const int n_frames,
                const std::string name)
: Encoder<B>(K, N, n_frames, name), m((int)std::log2(N)), frozen_bits(frozen_bits)
{
	if (this->N != (int)frozen_bits.size())
	{
		std::stringstream message;
		message << "'frozen_bits.size()' has to be equal to 'N' ('frozen_bits.size()' = " << frozen_bits.size()
		        << ", 'N' = " << N << ").";
		throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
	}

	auto k = 0; for (auto i = 0; i < this->N; i++) if (frozen_bits[i] == 0) k++;
	if (this->K != k)
	{
		std::stringstream message;
		message << "The number of information bits in the frozen_bits is invalid ('K' = " << K << ", 'k' = "
		        << k << ").";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}
}

template <typename B>
void Encoder_polar<B>
::_encode(const B *U_K, B *X_N, const int frame_id)
{
	this->convert(U_K, X_N);
	this->light_encode(X_N);
}

template <typename B>
void Encoder_polar<B>
::light_encode(B *bits)
{
	for (auto k = (this->N >> 1); k > 0; k >>= 1)
		for (auto j = 0; j < this->N; j += 2 * k)
			for (auto i = 0; i < k; i++)
				bits[j + i] = bits[j + i] ^ bits[k + j + i];
}

template <typename B>
void Encoder_polar<B>
::convert(const B *U_K, B *U_N)
{
	if (U_K == U_N)
	{
		std::vector<B> U_K_tmp(this->K);
		std::copy(U_K, U_K + this->K, U_K_tmp.begin());

		auto j = 0;
		for (unsigned i = 0; i < frozen_bits.size(); i++)
			U_N[i] = (frozen_bits[i]) ? (B)0 : U_K_tmp[j++];
	}
	else
	{
		auto j = 0;
		for (unsigned i = 0; i < frozen_bits.size(); i++)
			U_N[i] = (frozen_bits[i]) ? (B)0 : U_K[j++];
	}
}

// ==================================================================================== explicit template instantiation 
#include "Tools/types.h"
#ifdef MULTI_PREC
template class aff3ct::module::Encoder_polar<B_8>;
template class aff3ct::module::Encoder_polar<B_16>;
template class aff3ct::module::Encoder_polar<B_32>;
template class aff3ct::module::Encoder_polar<B_64>;
#else
template class aff3ct::module::Encoder_polar<B>;
#endif
// ==================================================================================== explicit template instantiation
