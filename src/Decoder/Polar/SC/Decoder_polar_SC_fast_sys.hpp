#ifndef DECODER_POLAR_SC_FAST_SYS_
#define DECODER_POLAR_SC_FAST_SYS_

#include <map>
#include <vector>
#include "../../../Tools/MIPP/mipp.h"

#include "../decoder_polar_functions.h"
#include "../../Decoder.hpp"

template <typename B, typename R, class API_polar>
class Decoder_polar_SC_fast_sys : public Decoder<B,R>
{
protected:
	const int             N;            // n bits
	const int             m;            // graph depth
	const int             n_frames;     // number of frames to decode in parallel
	      mipp::vector<R> l;            // lambda, LR or LLR
	      mipp::vector<B> s;            // bits, partial sums
	const mipp::vector<B> &frozen_bits; // frozen bits

	std::vector<char> pattern_types_per_id;

public:
	Decoder_polar_SC_fast_sys(const int& N, const mipp::vector<B>& frozen_bits);
	virtual ~Decoder_polar_SC_fast_sys();

	        void load  (const mipp::vector<R>& Y_N);
	virtual void decode(                          );
	        void store (      mipp::vector<B>& V_N) const;
	        void unpack(      mipp::vector<B>& V_N) const;

	int get_n_frames() const;

protected:
	virtual void recursive_decode(const int off_l, const int off_s, const int reverse_depth, int &id);
};

#include "Decoder_polar_SC_fast_sys.hxx"

#endif /* DECODER_POLAR_SC_FAST_SYS_ */