#ifndef FACTORY_FROZENBITS_GENERATOR_HPP
#define FACTORY_FROZENBITS_GENERATOR_HPP

#include "Tools/Code/Polar/Frozenbits_generator/Frozenbits_generator.hpp"

#include "../../../Factory.hpp"

namespace aff3ct
{
namespace factory
{
struct Frozenbits_generator : public Factory
{
	static const std::string name;
	static const std::string prefix;

	class parameters : public Factory::parameters
	{
	public:
		// ------------------------------------------------------------------------------------------------- PARAMETERS
		// required parameters
		int         K       = -1;
		int         N_cw    = -1;

		// optional parameters
		std::string type    = "GA";
		std::string path_fb = "../conf/cde/awgn_polar_codes/TV";
		std::string path_pb = "../lib/polar_bounds/bin/polar_bounds";
		float       sigma   = -1.f;

		// ---------------------------------------------------------------------------------------------------- METHODS
		parameters(const std::string p = Frozenbits_generator::prefix);
		virtual ~parameters();
		Frozenbits_generator::parameters* clone() const;

		// parameters construction
		void get_description(arg_map &req_args, arg_map &opt_args                              ) const;
		void store          (const arg_val_map &vals                                           );
		void get_headers    (std::map<std::string,header_list>& headers, const bool full = true) const;

		// builder
		tools::Frozenbits_generator* build() const;
	};

	static tools::Frozenbits_generator* build(const parameters &params);
};
}
}

#endif /* FACTORY_FROZENBITS_GENERATOR_HPP */
