#include <cmath>
#include <complex>
#include <limits>

#include "Tools/Exception/exception.hpp"

#include "Modem_PAM.hpp"

namespace aff3ct
{
namespace module
{
/*
 * Constructor / Destructor
 */
template <typename B, typename R, typename Q, tools::proto_max<Q> MAX>
Modem_PAM<B,R,Q,MAX>
::Modem_PAM(const int N, const R sigma, const int bits_per_symbol, const bool disable_sig2, const int n_frames,
            const std::string name)
: Modem<B,R,Q>(N,
               (int)std::ceil((float)N / (float)bits_per_symbol),
               sigma,
               n_frames,
               name),
  bits_per_symbol(bits_per_symbol),
  nbr_symbols    (1 << bits_per_symbol),
  sqrt_es        ((R)std::sqrt((this->nbr_symbols * this->nbr_symbols - 1.0) / 3.0)),
  disable_sig2   (disable_sig2),
  constellation  (nbr_symbols)
{
	std::vector<B> bits(this->bits_per_symbol);

	for (auto j = 0; j < this->nbr_symbols; j++)
	{
		for (auto l = 0; l < this->bits_per_symbol; l++)
			bits[l] = (j >> l) & 1;

		this->constellation[j] = this->bits_to_symbol(&bits[0]);
	}
}

template <typename B, typename R, typename Q, tools::proto_max<Q> MAX>
Modem_PAM<B,R,Q,MAX>
::~Modem_PAM()
{
}

/*
 * Mapping function
 */
template <typename B, typename R, typename Q, tools::proto_max<Q> MAX>
R Modem_PAM<B,R,Q,MAX>
::bits_to_symbol(const B* bits) const
 {
	auto bps = this->bits_per_symbol;

	auto symbol = (R)1.0 - ((R)bits[0] + (R)bits[0]);
	for (auto j = 1; j < bps; j++)
		symbol = ((R)1.0 - ((R)bits[j] + (R)bits[j])) * ((1 << j) - symbol);

	return symbol / this->sqrt_es;
 }

/*
 * Modem
 */
template <typename B,typename R, typename Q, tools::proto_max<Q> MAX>
void Modem_PAM<B,R,Q,MAX>
::_modulate(const B *X_N1, R *X_N2, const int frame_id)
{
	auto size_in  = this->N;
	auto size_out = this->N_mod;
	auto bps      = this->bits_per_symbol;

	auto main_loop_size = size_in / bps;
	for (auto i = 0; i < main_loop_size; i++)
	{
		// compute the symbol "on the fly"
		// auto symbol = bits_to_symbol(&X_N1[i*bps]);

		// determine the symbol with a lookup table
		unsigned idx = 0;
		for (auto j = 0; j < bps; j++)
			idx += unsigned(unsigned(1 << j) * X_N1[i * bps +j]);
		auto symbol = this->constellation[idx];

		X_N2[i] = symbol;
	}

	// last elements if "size_in" is not a multiple of the number of bits per symbol
	if (main_loop_size * bps < size_in)
	{
		unsigned idx = 0;
		for (auto j = 0; j < size_in - (main_loop_size * bps); j++)
			idx += unsigned(unsigned(1 << j) * X_N1[main_loop_size * bps +j]);
		auto symbol = this->constellation[idx];

		X_N2[size_out -1] = symbol;
	}
}

/*
 * Filter
 */
template <typename B,typename R, typename Q, tools::proto_max<Q> MAX>
void Modem_PAM<B,R,Q,MAX>
::_filter(const R *Y_N1, R *Y_N2, const int frame_id)
{
	std::copy(Y_N1, Y_N1 + this->N_fil, Y_N2);
}

/*
 * Demodulator
 */
template <typename B,typename R, typename Q, tools::proto_max<Q> MAX>
void Modem_PAM<B,R,Q,MAX>
::_demodulate(const Q *Y_N1, Q *Y_N2, const int frame_id)
{
	if (typeid(R) != typeid(Q))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'R' and 'Q' have to be the same.");

	if (typeid(Q) != typeid(float) && typeid(Q) != typeid(double))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'Q' has to be float or double.");

	auto size       = this->N;
	auto inv_sigma2 = disable_sig2 ? (Q)1.0 : (Q)(1.0 / (2 * this->sigma * this->sigma));

	for (auto n = 0; n < size; n++) // loop upon the LLRs
	{
		auto L0 = -std::numeric_limits<Q>::infinity();
		auto L1 = -std::numeric_limits<Q>::infinity();
		auto b  = n % this->bits_per_symbol; // bit position in the symbol
		auto k  = n / this->bits_per_symbol; // symbol position

		for (auto j = 0; j < this->nbr_symbols; j++)
			if ((j & (1 << b)) == 0)
				L0 = MAX(L0, -(Y_N1[k] - (Q)this->constellation[j]) *
				              (Y_N1[k] - (Q)this->constellation[j]) * inv_sigma2);
			else
				L1 = MAX(L1, -(Y_N1[k] - (Q)this->constellation[j]) *
				              (Y_N1[k] - (Q)this->constellation[j]) * inv_sigma2);

		Y_N2[n] = (L0 - L1);
	}
}

/*
 * Demodulator
 */
template <typename B,typename R, typename Q, tools::proto_max<Q> MAX>
void Modem_PAM<B,R,Q,MAX>
::_demodulate_wg(const R *H_N, const Q *Y_N1, Q *Y_N2, const int frame_id)
{
	if (typeid(R) != typeid(Q))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'R' and 'Q' have to be the same.");

	if (typeid(Q) != typeid(float) && typeid(Q) != typeid(double))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'Q' has to be float or double.");

	auto size       = this->N;
	auto inv_sigma2 = disable_sig2 ? (Q)1.0 : (Q)(1.0 / (2 * this->sigma * this->sigma));

	for (auto n = 0; n < size; n++) // loop upon the LLRs
	{
		auto L0 = -std::numeric_limits<Q>::infinity();
		auto L1 = -std::numeric_limits<Q>::infinity();
		auto b  = n % this->bits_per_symbol; // bit position in the symbol
		auto k  = n / this->bits_per_symbol; // symbol position

		for (auto j = 0; j < this->nbr_symbols; j++)
			if ((j & (1 << b)) == 0)
				L0 = MAX(L0, -(Y_N1[k] - (Q)H_N[k] * (Q)this->constellation[j]) *
				              (Y_N1[k] - (Q)H_N[k] * (Q)this->constellation[j]) * inv_sigma2);
			else
				L1 = MAX(L1, -(Y_N1[k] - (Q)H_N[k] * (Q)this->constellation[j]) *
				              (Y_N1[k] - (Q)H_N[k] * (Q)this->constellation[j]) * inv_sigma2);

		Y_N2[n] = (L0 - L1);
	}
}

template <typename B,typename R, typename Q, tools::proto_max<Q> MAX>
void Modem_PAM<B,R,Q,MAX>
::_tdemodulate(const Q *Y_N1, const Q *Y_N2, Q *Y_N3, const int frame_id)
{
	if (typeid(R) != typeid(Q))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'R' and 'Q' have to be the same.");

	if (typeid(Q) != typeid(float) && typeid(Q) != typeid(double))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'Q' has to be float or double.");

	auto size       = this->N;
	auto inv_sigma2 = disable_sig2 ? (Q)1.0 : (Q)1.0 / (2 * this->sigma * this->sigma);

	for (auto n = 0; n < size; n++) // loop upon the LLRs
	{
		auto L0 = -std::numeric_limits<Q>::infinity();
		auto L1 = -std::numeric_limits<Q>::infinity();
		auto b  = n % this->bits_per_symbol; // bit position in the symbol
		auto k  = n / this->bits_per_symbol; // symbol position

		for (auto j = 0; j < this->nbr_symbols; j++)
		{
			auto tempL  = (Q)((Y_N1[k] - this->constellation[j]) *
			                  (Y_N1[k] - this->constellation[j]) * inv_sigma2);

			for (auto l = 0; l < b; l++)
				tempL += (j & (1 << l)) * Y_N2[k * this->bits_per_symbol +l];

			for (auto l = b +1; l < this->bits_per_symbol; l++)
				tempL += (j & (1 << l)) * Y_N2[k * this->bits_per_symbol +l];

			if ((j & (1 << b)) == 0)
				L0 = MAX(L0, -tempL);
			else
				L1 = MAX(L1, -tempL);
		}

		Y_N3[n] = (L0 - L1);
	}
}

template <typename B,typename R, typename Q, tools::proto_max<Q> MAX>
void Modem_PAM<B,R,Q,MAX>
::_tdemodulate_wg(const R *H_N, const Q *Y_N1, const Q *Y_N2, Q *Y_N3, const int frame_id)
{
	if (typeid(R) != typeid(Q))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'R' and 'Q' have to be the same.");

	if (typeid(Q) != typeid(float) && typeid(Q) != typeid(double))
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Type 'Q' has to be float or double.");

	auto size       = this->N;
	auto inv_sigma2 = disable_sig2 ? (Q)1.0 : (Q)1.0 / (2 * this->sigma * this->sigma);

	for (auto n = 0; n < size; n++) // boucle sur les LLRs
	{
		auto L0 = -std::numeric_limits<Q>::infinity();
		auto L1 = -std::numeric_limits<Q>::infinity();
		auto b  = n % this->bits_per_symbol; // bit position in the symbol
		auto k  = n / this->bits_per_symbol; // symbol position

		for (auto j = 0; j < this->nbr_symbols; j++)
		{
			auto tempL = (Q)((Y_N1[k] - (Q)H_N[k] * this->constellation[j]) *
			                 (Y_N1[k] - (Q)H_N[k] * this->constellation[j]) * inv_sigma2);

			for (auto l = 0; l < b; l++)
				tempL += (j & (1 << l)) * Y_N2[k * this->bits_per_symbol +l];

			for (auto l = b +1; l < this->bits_per_symbol; l++)
				tempL += (j & (1 << l)) * Y_N2[k * this->bits_per_symbol +l];

			if ((j & (1 << b)) == 0)
				L0 = MAX(L0, -tempL);
			else
				L1 = MAX(L1, -tempL);
		}

		Y_N3[n] = (L0 - L1);
	}
}
}
}
