#ifdef _MSC_VER
#include <iterator>
#endif
#include <sstream>

#include "Tools/Exception/exception.hpp"

#include "Encoder_RSC_sys.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename B>
Encoder_RSC_sys<B>
::Encoder_RSC_sys(const int& K, const int& N, const int n_ff, const int& n_frames, const bool buffered_encoding,
                  const std::string name)
: Encoder<B>(K, N, n_frames, name), n_ff(n_ff), n_states(1 << n_ff),
  buffered_encoding(buffered_encoding)
{
	if (n_ff <= 0)
	{
		std::stringstream message;
		message << "'n_ff' has to be greater than 0 ('n_ff' = " << n_ff << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	if (N - 2 * n_ff !=  2 * K)
	{
		std::stringstream message;
		message << "'N' - 2 * 'n_ff' has to be equal to 2 * 'K' ('N' = " << N << ", 'n_ff' = " << n_ff
		        << ", 'K' = " << K << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}
}

template <typename B>
int Encoder_RSC_sys<B>
::get_n_ff()
{
	return n_ff;
}
template <typename B>
int Encoder_RSC_sys<B>
::tail_length() const
{
	return 2 * n_ff;
}

template <typename B>
void Encoder_RSC_sys<B>
::_encode(const B *U_K, B *X_N, const int frame_id)
{
	if (buffered_encoding)
		__encode(U_K,
		         X_N,                             // sys
		         X_N + 1 * this->K,               // tail sys
		         X_N + 1 * this->K + this->n_ff,  // par
		         X_N + 2 * this->K + this->n_ff); // tail par
	else
		__encode(U_K,
		         X_N,                   // sys
		         X_N + 2 * this->K,     // tail sys
		         X_N + 1,               // par
		         X_N + 2 * this->K + 1, // tail par
		         2,                     // stride
		         2);                    // stride tail bits
}

template <typename B>
std::vector<std::vector<int>> Encoder_RSC_sys<B>
::get_trellis()
{
	std::vector<std::vector<int>> trellis(10, std::vector<int>(this->n_states));

	std::vector<bool> occurrence(this->n_states, false);

	for (auto i = 0; i < this->n_states; i++)
	{
		// sys = 0
		auto state   = i;
		auto bit_sys = 0;
		auto bit_par = inner_encode(bit_sys, state);

		trellis[0 + (occurrence[state] ? 3 : 0)][state] = i;                 // initial state
		trellis[1 + (occurrence[state] ? 3 : 0)][state] = +1;                // gamma coeff
		trellis[2 + (occurrence[state] ? 3 : 0)][state] = bit_sys ^ bit_par; // gamma
		trellis[6                              ][i    ] = state;             // final state, bit syst = 0
		trellis[7                              ][i    ] = bit_sys ^ bit_par; // gamma      , bit syst = 0

		occurrence[state] = true;

		// sys = 1
		state   = i;
		bit_sys = 1;
		bit_par = inner_encode(bit_sys, state);

		trellis[0 + (occurrence[state] ? 3 : 0)][state] = i;                 // initial state
		trellis[1 + (occurrence[state] ? 3 : 0)][state] = -1;                // gamma coeff
		trellis[2 + (occurrence[state] ? 3 : 0)][state] = bit_sys ^ bit_par; // gamma
		trellis[8                              ][i    ] = state;             // initial state, bit syst = 1
		trellis[9                              ][i    ] = bit_sys ^ bit_par; // gamma        , bit syst = 1

		occurrence[state] = true;
	}

	return trellis;
}

template <typename B>
void Encoder_RSC_sys<B>
::__encode(const B* U_K, B* sys, B* tail_sys, B* par, B* tail_par, const int stride, const int stride_tail)
{
	auto state = 0; // initial (and final) state 0 0 0

	if (sys != nullptr)
		for (auto i = 0; i < this->K; i++)
			sys[i * stride] = U_K[i];

	// standard frame encoding process
	if (par != nullptr)
		for (auto i = 0; i < this->K; i++)
			par[i * stride] = inner_encode((int)U_K[i], state); // encoding block
	else
		for (auto i = 0; i < this->K; i++)
			inner_encode((int)U_K[i], state); // encoding block

	// tail bits for initialization conditions (state of data "state" have to be 0 0 0)
	for (auto i = 0; i < this->n_ff; i++)
	{
		B bit_sys = tail_bit_sys(state);

		if (tail_sys != nullptr)
			tail_sys[i * stride_tail] = bit_sys; // systematic transmission of the bit

		auto p = inner_encode((int)bit_sys, state); // encoding block

		if (tail_par != nullptr)
			tail_par[i * stride_tail] = p;
	}

	if (state != 0)
	{
		std::stringstream message;
		message << "'state' should be equal to 0 ('state' = " <<  state << ").";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}
}

// ==================================================================================== explicit template instantiation 
#include "Tools/types.h"
#ifdef MULTI_PREC
template class aff3ct::module::Encoder_RSC_sys<B_8>;
template class aff3ct::module::Encoder_RSC_sys<B_16>;
template class aff3ct::module::Encoder_RSC_sys<B_32>;
template class aff3ct::module::Encoder_RSC_sys<B_64>;
#else
template class aff3ct::module::Encoder_RSC_sys<B>;
#endif
// ==================================================================================== explicit template instantiation
