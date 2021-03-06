#ifndef FACTORY_DECODER_TURBO_DB_HPP
#define FACTORY_DECODER_TURBO_DB_HPP

#include <string>
#include <type_traits>

#include "Module/Decoder/RSC_DB/BCJR//Decoder_RSC_DB_BCJR.hpp"
#include "Module/Decoder/Turbo_DB/Decoder_turbo_DB.hpp"
#include "Module/Interleaver/Interleaver.hpp"

#include "Factory/Module/Interleaver/Interleaver.hpp"
#include "Factory/Tools/Code/Turbo/Flip_and_check_DB.hpp"
#include "Factory/Tools/Code/Turbo/Scaling_factor.hpp"
#include "Factory/Module/Decoder/RSC_DB/Decoder_RSC_DB.hpp"

#include "../Decoder.hpp"

namespace aff3ct
{
namespace factory
{
struct Decoder_turbo_DB : public Decoder
{
	static const std::string name;
	static const std::string prefix;

	class parameters : public Decoder::parameters
	{
	public:
		// ------------------------------------------------------------------------------------------------- PARAMETERS
		// optional parameters
		int n_ite = 6;

		// depending parameters
		Decoder_RSC_DB   ::parameters *sub;
		Interleaver      ::parameters *itl;
		Scaling_factor   ::parameters *sf;
		Flip_and_check_DB::parameters *fnc;

		// ---------------------------------------------------------------------------------------------------- METHODS
		parameters(const std::string p = Decoder_turbo_DB::prefix);
		virtual ~parameters();
		Decoder_turbo_DB::parameters* clone() const;

		virtual std::vector<std::string> get_names      () const;
		virtual std::vector<std::string> get_short_names() const;
		virtual std::vector<std::string> get_prefixes   () const;

		// parameters construction
		void get_description(arg_map &req_args, arg_map &opt_args                              ) const;
		void store          (const arg_val_map &vals                                           );
		void get_headers    (std::map<std::string,header_list>& headers, const bool full = true) const;

		// builder
		template <typename B = int, typename Q = float>
		module::Decoder_turbo_DB<B,Q>* build(const module::Interleaver<Q>           &itl,
		                                           module::Decoder_RSC_DB_BCJR<B,Q> &siso_n,
		                                           module::Decoder_RSC_DB_BCJR<B,Q> &siso_i) const;
	};

	template <typename B = int, typename Q = float>
	static module::Decoder_turbo_DB<B,Q>* build(const parameters                       &params,
	                                            const module::Interleaver<Q>           &itl,
	                                                  module::Decoder_RSC_DB_BCJR<B,Q> &siso_n,
	                                                  module::Decoder_RSC_DB_BCJR<B,Q> &siso_i);

	static void build_args(arg_map &req_args, arg_map &opt_args, const std::string p = prefix);
	static void store_args(const arg_val_map &vals, parameters &params, const std::string p = prefix);
	static void make_header(header_list& head_dec, header_list& head_itl, const parameters& params, const bool full = true);
};
}
}

#endif /* FACTORY_DECODER_TURBO_DB_HPP */
