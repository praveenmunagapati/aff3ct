#ifndef FACTORY_CODEC_TURBO_HPP
#define FACTORY_CODEC_TURBO_HPP

#include <string>
#include <cmath>

#include "Factory/Module/Encoder/Turbo/Encoder_turbo.hpp"
#include "Factory/Module/Decoder/Turbo/Decoder_turbo.hpp"
#include "Factory/Module/Puncturer/Turbo/Puncturer_turbo.hpp"

#include "Module/CRC/CRC.hpp"
#include "Module/Codec/Turbo/Codec_turbo.hpp"

#include "../Codec_SIHO.hpp"

namespace aff3ct
{
namespace factory
{
struct Codec_turbo : public Codec_SIHO
{
	static const std::string name;
	static const std::string prefix;

	class parameters : public Codec_SIHO::parameters
	{
	public:
		// ------------------------------------------------------------------------------------------------- PARAMETERS
		// depending parameters
		Encoder_turbo  ::parameters<> *enc;
		Decoder_turbo  ::parameters<> *dec;
		Puncturer_turbo::parameters   *pct;

		// ---------------------------------------------------------------------------------------------------- METHODS
		parameters(const std::string p = Codec_turbo::prefix);
		virtual ~parameters();
		Codec_turbo::parameters* clone() const;
		void enable_puncturer();

		virtual std::vector<std::string> get_names      () const;
		virtual std::vector<std::string> get_short_names() const;
		virtual std::vector<std::string> get_prefixes   () const;

		// parameters construction
		void get_description(arg_map &req_args, arg_map &opt_args                              ) const;
		void store          (const arg_val_map &vals                                           );
		void get_headers    (std::map<std::string,header_list>& headers, const bool full = true) const;

		// builder
		template <typename B = int, typename Q = float>
		module::Codec_turbo<B,Q>* build(module::CRC<B> *crc = nullptr) const;
	};

	template <typename B = int, typename Q = float>
	static module::Codec_turbo<B,Q>* build(const parameters &params, module::CRC<B> *crc = nullptr);
};
}
}

#endif /* FACTORY_CODEC_TURBO_HPP */
