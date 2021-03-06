#ifndef DECODER_RSC_DB_BCJR_DVB_RCS2_HPP_
#define DECODER_RSC_DB_BCJR_DVB_RCS2_HPP_

#include <vector>

#include "Tools/Math/max.h"

#include "Decoder_RSC_DB_BCJR.hpp"

namespace aff3ct
{
namespace module
{
template <typename B = int, typename R = float, tools::proto_max<R> MAX = tools::max>
class Decoder_RSC_DB_BCJR_DVB_RCS2 : public Decoder_RSC_DB_BCJR<B,R>
{
public:
	Decoder_RSC_DB_BCJR_DVB_RCS2(const int K,
	                             const std::vector<std::vector<int>> &trellis,
	                             const bool buffered_encoding = true,
	                             const int n_frames = 1,
	                             const std::string name = "Decoder_RSC_DB_BCJR_DVB_RCS2");
	virtual ~Decoder_RSC_DB_BCJR_DVB_RCS2();

protected:
	void __fwd_recursion(const R *sys, const R *par        );
	void __bwd_recursion(const R *sys, const R *par, R* ext);
};
}
}
#include "Decoder_RSC_DB_BCJR_DVB_RCS2.hxx"

#endif /* DECODER_RSC_DB_BCJR_DVB_RCS2_HPP_ */
