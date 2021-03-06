#ifndef CODEC_POLAR_HPP_
#define CODEC_POLAR_HPP_

#include "Tools/Code/Polar/Frozenbits_generator/Frozenbits_generator.hpp"
#include "Tools/Code/Polar/Frozenbits_notifier.hpp"

#include "Module/Puncturer/Polar/Puncturer_polar_wangliu.hpp"

#include "Factory/Module/Encoder/Polar/Encoder_polar.hpp"
#include "Factory/Module/Decoder/Polar/Decoder_polar.hpp"
#include "Factory/Module/Puncturer/Polar/Puncturer_polar.hpp"
#include "Factory/Tools/Code/Polar/Frozenbits_generator.hpp"

#include "../Codec_SISO_SIHO.hpp"

namespace aff3ct
{
namespace module
{
template <typename B = int, typename Q = float>
class Codec_polar : public Codec_SISO_SIHO<B,Q>
{
protected:
	const bool adaptive_fb;
	std::vector<bool> frozen_bits; // known bits (alias frozen bits) are set to true
	const bool generated_decoder;
	tools::Frozenbits_generator *fb_generator;
	Puncturer_polar_wangliu<B,Q> *puncturer_wangliu;
	tools::Frozenbits_notifier *fb_decoder;

public:
	Codec_polar(const factory::Frozenbits_generator::parameters &fb_par,
	            const factory::Encoder_polar       ::parameters &enc_par,
	            const factory::Decoder_polar       ::parameters &dec_par,
	            const factory::Puncturer_polar     ::parameters *pct_par = nullptr,
	            CRC<B>* crc = nullptr, const std::string name = "Codec_polar");
	virtual ~Codec_polar();

	void set_sigma(const float sigma);

protected:
	void _extract_sys_par(const Q *Y_N, Q *sys, Q *par, const int frame_id);
	void _extract_sys_llr(const Q *Y_N, Q *sys,         const int frame_id);
	void _add_sys_ext    (const Q *ext, Q *Y_N,         const int frame_id);
};
}
}

#endif /* CODEC_POLAR_HPP_ */
