#include <cassert>
#include <vector>
#include <cmath>

#include "Encoder_polar.hpp"

template <typename B>
Encoder_polar<B>
::Encoder_polar(const int& K, const int& N, const mipp::vector<B>& frozen_bits, const int n_frames)
: K(K), N(N), m(log2(N)), n_frames(n_frames), frozen_bits(frozen_bits)
{
	assert(frozen_bits.size() == (unsigned) N);
	assert(n_frames > 0);
}

template <typename B>
void Encoder_polar<B>
::encode(const mipp::vector<B>& U_K, mipp::vector<B>& X_N)
{
	assert(U_K.size() == (unsigned) (K * n_frames));
	assert(X_N.size() == (unsigned) (N * n_frames));

	mipp::vector<B> U_N(N * n_frames);
	this->convert(U_K, U_N);

	for (auto i_frame = 0; i_frame < this->n_frames; i_frame++)
		frame_encode(U_N, X_N, i_frame); // frame encode
}

template <typename B>
void Encoder_polar<B>
::encode(const mipp::vector<B>& U_K, mipp::vector<B>& U_N, mipp::vector<B>& X_N)
{
	assert(U_K.size() == (unsigned) (K * n_frames));
	assert(X_N.size() == (unsigned) (N * n_frames));

	this->convert(U_K, U_N);

	for (auto i_frame = 0; i_frame < this->n_frames; i_frame++)
		frame_encode(U_N, X_N, i_frame); // frame encode
}

template <typename B>
void Encoder_polar<B>
::frame_encode(const mipp::vector<B>& U_N, mipp::vector<B>& X_N, const int &i_frame)
{
	const auto offset = i_frame * N;

	for (auto i = 0; i < N; i++) X_N[offset +i] = U_N[offset +i];

	light_encode(X_N.data() + offset);
}

template <typename B>
void Encoder_polar<B>
::light_encode(B *bits)
{
	for (auto k = (N >> 1); k > 0; k >>= 1)
		for (auto j = 0; j < N; j += 2 * k)
			for (auto i = 0; i < k; i++)
				bits[j + i] = bits[j + i] ^ bits[k + j + i];
}

template <typename B>
void Encoder_polar<B>
::convert(const mipp::vector<B>& U_K, mipp::vector<B>& U_N)
{
	assert(U_K.size() == (unsigned) (K * n_frames));
	assert(U_N.size() == (unsigned) (N * n_frames));

	for (auto f = 0; f < n_frames; f++)
	{
		const auto offset_U_k = f * K;
		const auto offset_U_n = f * N;

		auto j = 0;
		for (unsigned i = 0; i < frozen_bits.size(); i++)
			U_N[offset_U_n +i] = (frozen_bits[i]) ? (B)0 : U_K[offset_U_k + j++];
	}
}

// ==================================================================================== explicit template instantiation 
#include "../../Tools/types.h"
#ifdef MULTI_PREC
template class Encoder_polar<B_8>;
template class Encoder_polar<B_16>;
template class Encoder_polar<B_32>;
template class Encoder_polar<B_64>;
#else
template class Encoder_polar<B>;
#endif
// ==================================================================================== explicit template instantiation