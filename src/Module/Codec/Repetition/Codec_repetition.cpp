#include "Tools/Exception/exception.hpp"

#include "Factory/Module/Puncturer/Puncturer.hpp"

#include "Codec_repetition.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename B, typename Q>
Codec_repetition<B,Q>
::Codec_repetition(const factory::Encoder_repetition::parameters &enc_params,
                   const factory::Decoder_repetition::parameters &dec_params,
                   const std::string name)
: Codec     <B,Q>(enc_params.K, enc_params.N_cw, enc_params.N_cw, enc_params.tail_length, enc_params.n_frames, name),
  Codec_SIHO<B,Q>(enc_params.K, enc_params.N_cw, enc_params.N_cw, enc_params.tail_length, enc_params.n_frames, name)
{
	// ----------------------------------------------------------------------------------------------------- exceptions
	if (enc_params.K != dec_params.K)
	{
		std::stringstream message;
		message << "'enc_params.K' has to be equal to 'dec_params.K' ('enc_params.K' = " << enc_params.K
		        << ", 'dec_params.K' = " << dec_params.K << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	if (enc_params.N_cw != dec_params.N_cw)
	{
		std::stringstream message;
		message << "'enc_params.N_cw' has to be equal to 'dec_params.N_cw' ('enc_params.N_cw' = " << enc_params.N_cw
		        << ", 'dec_params.N_cw' = " << dec_params.N_cw << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	if (enc_params.n_frames != dec_params.n_frames)
	{
		std::stringstream message;
		message << "'enc_params.n_frames' has to be equal to 'dec_params.n_frames' ('enc_params.n_frames' = "
		        << enc_params.n_frames << ", 'dec_params.n_frames' = " << dec_params.n_frames << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	// ---------------------------------------------------------------------------------------------------- allocations
	factory::Puncturer::parameters pct_params;
	pct_params.type     = "NO";
	pct_params.K        = enc_params.K;
	pct_params.N        = enc_params.N_cw;
	pct_params.N_cw     = enc_params.N_cw;
	pct_params.n_frames = enc_params.n_frames;

	this->set_puncturer(factory::Puncturer::build<B,Q>(pct_params));

	try
	{
		this->set_encoder(factory::Encoder_repetition::build<B>(enc_params));
	}
	catch (tools::cannot_allocate const&)
	{
		this->set_encoder(factory::Encoder::build<B>(enc_params));
	}

	this->set_decoder_siho(factory::Decoder_repetition::build<B,Q>(dec_params));
}

template <typename B, typename Q>
Codec_repetition<B,Q>
::~Codec_repetition()
{
}

// ==================================================================================== explicit template instantiation 
#include "Tools/types.h"
#ifdef MULTI_PREC
template class aff3ct::module::Codec_repetition<B_8,Q_8>;
template class aff3ct::module::Codec_repetition<B_16,Q_16>;
template class aff3ct::module::Codec_repetition<B_32,Q_32>;
template class aff3ct::module::Codec_repetition<B_64,Q_64>;
#else
template class aff3ct::module::Codec_repetition<B,Q>;
#endif
// ==================================================================================== explicit template instantiation
