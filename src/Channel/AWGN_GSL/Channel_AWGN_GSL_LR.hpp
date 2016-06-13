#ifdef CHANNEL_GSL

#ifndef CHANNEL_AWGN_GSL_LR_HPP_
#define CHANNEL_AWGN_GSL_LR_HPP_

#include <vector>
#include "../../Tools/MIPP/mipp.h"

#include "Channel_AWGN_GSL.hpp"

template <typename B, typename R>
class Channel_AWGN_GSL_LR : public Channel_AWGN_GSL<B,R>
{
public:
	Channel_AWGN_GSL_LR(const R& sigma, const int seed = 0, const R& scaling_factor = 1);

	virtual ~Channel_AWGN_GSL_LR();

	void add_noise(const mipp::vector<B>& X_N, mipp::vector<R>& Y_N);
};

#endif // CHANNEL_AWGN_GSL_LR_HPP_

#endif