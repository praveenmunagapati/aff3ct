#include "LDPC/Codec_LDPC.hpp"
#include "Polar/Codec_polar.hpp"
#include "RSC/Codec_RSC.hpp"
#include "RSC_DB/Codec_RSC_DB.hpp"
#include "Uncoded/Codec_uncoded.hpp"

#include "Codec_SISO.hpp"

using namespace aff3ct;
using namespace aff3ct::factory;

const std::string aff3ct::factory::Codec_SISO::name   = "Codec SISO";
const std::string aff3ct::factory::Codec_SISO::prefix = "cdc";

Codec_SISO::parameters
::parameters(const std::string prefix)
: Codec::parameters(Codec_SISO::name, prefix)
{
}

Codec_SISO::parameters
::parameters(const std::string name, const std::string prefix)
: Codec::parameters(name, prefix)
{
}

Codec_SISO::parameters
::~parameters()
{
}

template <typename B, typename Q>
module::Codec_SISO<B,Q>* Codec_SISO::parameters
::build(module::CRC<B>* crc) const
{
	     if (get_name() == "Codec LDPC"   ) return dynamic_cast<const Codec_LDPC   ::parameters&>(*this).template build<B,Q>(crc);
	else if (get_name() == "Codec Polar"  ) return dynamic_cast<const Codec_polar  ::parameters&>(*this).template build<B,Q>(crc);
	else if (get_name() == "Codec RSC"    ) return dynamic_cast<const Codec_RSC    ::parameters&>(*this).template build<B,Q>(crc);
	else if (get_name() == "Codec RSC DB" ) return dynamic_cast<const Codec_RSC_DB ::parameters&>(*this).template build<B,Q>(crc);
	else if (get_name() == "Codec Uncoded") return dynamic_cast<const Codec_uncoded::parameters&>(*this).template build<B,Q>(crc);

	throw tools::cannot_allocate(__FILE__, __LINE__, __func__);
}

template <typename B, typename Q>
module::Codec_SISO<B,Q>* Codec_SISO
::build(const parameters &params, module::CRC<B> *crc)
{
	return params.template build<B,Q>(crc);
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef MULTI_PREC
template aff3ct::module::Codec_SISO<B_8 ,Q_8 >* aff3ct::factory::Codec_SISO::parameters::build<B_8 ,Q_8 >(aff3ct::module::CRC<B_8 >*) const;
template aff3ct::module::Codec_SISO<B_16,Q_16>* aff3ct::factory::Codec_SISO::parameters::build<B_16,Q_16>(aff3ct::module::CRC<B_16>*) const;
template aff3ct::module::Codec_SISO<B_32,Q_32>* aff3ct::factory::Codec_SISO::parameters::build<B_32,Q_32>(aff3ct::module::CRC<B_32>*) const;
template aff3ct::module::Codec_SISO<B_64,Q_64>* aff3ct::factory::Codec_SISO::parameters::build<B_64,Q_64>(aff3ct::module::CRC<B_64>*) const;
template aff3ct::module::Codec_SISO<B_8 ,Q_8 >* aff3ct::factory::Codec_SISO::build<B_8 ,Q_8 >(const aff3ct::factory::Codec_SISO::parameters&, aff3ct::module::CRC<B_8 >*);
template aff3ct::module::Codec_SISO<B_16,Q_16>* aff3ct::factory::Codec_SISO::build<B_16,Q_16>(const aff3ct::factory::Codec_SISO::parameters&, aff3ct::module::CRC<B_16>*);
template aff3ct::module::Codec_SISO<B_32,Q_32>* aff3ct::factory::Codec_SISO::build<B_32,Q_32>(const aff3ct::factory::Codec_SISO::parameters&, aff3ct::module::CRC<B_32>*);
template aff3ct::module::Codec_SISO<B_64,Q_64>* aff3ct::factory::Codec_SISO::build<B_64,Q_64>(const aff3ct::factory::Codec_SISO::parameters&, aff3ct::module::CRC<B_64>*);
#else
template aff3ct::module::Codec_SISO<B,Q>* aff3ct::factory::Codec_SISO::parameters::build<B,Q>(aff3ct::module::CRC<B>*) const;
template aff3ct::module::Codec_SISO<B,Q>* aff3ct::factory::Codec_SISO::build<B,Q>(const aff3ct::factory::Codec_SISO::parameters&, aff3ct::module::CRC<B>*);
#endif
// ==================================================================================== explicit template instantiation
