#ifndef FACTORY_DECODER_LDPC_HPP
#define FACTORY_DECODER_LDPC_HPP

#include <string>

#include "Tools/Algo/Sparse_matrix/Sparse_matrix.hpp"

#include "Module/Decoder/Decoder_SIHO.hpp"
#include "Module/Decoder/Decoder_SISO_SIHO.hpp"

#include "../Decoder.hpp"

namespace aff3ct
{
namespace factory
{
struct Decoder_LDPC : public Decoder
{
	static const std::string name;
	static const std::string prefix;

	class parameters : public Decoder::parameters
	{
	public:
		// ------------------------------------------------------------------------------------------------- PARAMETERS
		// optional parameters
		std::string H_alist_path     = "";
		std::string simd_strategy    = "";
		float       norm_factor      = 1.f;
		float       offset           = 0.f;
		bool        enable_syndrome  = true;
		int         syndrome_depth   = 2;
		int         n_ite            = 10;

		// ---------------------------------------------------------------------------------------------------- METHODS
		parameters(const std::string p = Decoder_LDPC::prefix);
		virtual ~parameters();
		Decoder_LDPC::parameters* clone() const;

		// parameters construction
		void get_description(arg_map &req_args, arg_map &opt_args                              ) const;
		void store          (const arg_val_map &vals                                           );
		void get_headers    (std::map<std::string,header_list>& headers, const bool full = true) const;

		// builder
		template <typename B = int, typename Q = float>
		module::Decoder_SIHO<B,Q>* build(const tools::Sparse_matrix &H,
		                                 const std::vector<unsigned> &info_bits_pos) const;

		template <typename B = int, typename Q = float>
		module::Decoder_SISO_SIHO<B,Q>* build_siso(const tools::Sparse_matrix &H,
		                                           const std::vector<unsigned> &info_bits_pos) const;
	};

	template <typename B = int, typename Q = float>
	static module::Decoder_SIHO<B,Q>* build(const parameters& params, const tools::Sparse_matrix &H,
	                                        const std::vector<unsigned> &info_bits_pos);

	template <typename B = int, typename Q = float>
	static module::Decoder_SISO_SIHO<B,Q>* build_siso(const parameters& params, const tools::Sparse_matrix &H,
	                                                  const std::vector<unsigned> &info_bits_pos);
};
}
}

#endif /* FACTORY_DECODER_LDPC_HPP */
