#include <cmath>
#include <algorithm>

#include "Tools/Exception/exception.hpp"
#include "Tools/Display/bash_tools.h"
#include "Tools/Display/Statistics/Statistics.hpp"
#include "Tools/general_utils.h"
#include "Tools/Math/utils.h"

#include "EXIT.hpp"

using namespace aff3ct;
using namespace aff3ct::simulation;

template <typename B, typename R>
EXIT<B,R>
::EXIT(const factory::EXIT::parameters& params)
: Simulation(params),
  params(params),

  sig_a(0.f),
  sigma(0.f),
  ebn0 (0.f),
  esn0 (0.f),

  source   (nullptr),
  codec    (nullptr),
  modem    (nullptr),
  modem_a  (nullptr),
  channel  (nullptr),
  channel_a(nullptr),
  siso     (nullptr),
  monitor  (nullptr),
  terminal (nullptr)
{
#ifdef ENABLE_MPI
	std::clog << tools::format_warning("This simulation is not MPI ready, the same computations will be launched "
	                                   "on each MPI processes.") << std::endl;
#endif

	if (this->params.n_threads > 1)
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, "EXIT simu does not support the multi-threading.");

	this->modules["source"   ] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["codec"    ] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["encoder"  ] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["decoder"  ] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["modem"    ] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["modem_a"  ] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["channel"  ] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["channel_a"] = std::vector<module::Module*>(params.n_threads, nullptr);
	this->modules["monitor"  ] = std::vector<module::Module*>(params.n_threads, nullptr);

	this->monitor = this->build_monitor();

	this->modules["monitor"][0] = this->monitor;
}

template <typename B, typename R>
EXIT<B,R>
::~EXIT()
{
	if (monitor != nullptr) { delete monitor; monitor = nullptr; }
	release_objects();
}

template <typename B, typename R>
void EXIT<B,R>
::_build_communication_chain()
{
	release_objects();

	const auto N_mod = params.mdm->N_mod;
	const auto K_mod = factory::Modem::get_buffer_size_after_modulation(params.mdm->type,
	                                                                    params.cdc->K,
	                                                                    params.mdm->bps,
	                                                                    params.mdm->upf,
	                                                                    params.mdm->cpm_L);

	// build the objects
	source    = build_source   (     );
	codec     = build_codec    (     );
	modem     = build_modem    (     );
	modem_a   = build_modem_a  (     );
	channel   = build_channel  (N_mod);
	channel_a = build_channel_a(K_mod);
	terminal  = build_terminal (     );

	this->modules["source"   ][0] = source;
	this->modules["codec"    ][0] = codec;
	this->modules["encoder"  ][0] = codec->get_encoder();
	this->modules["decoder"  ][0] = codec->get_decoder_siso();
	this->modules["modem"    ][0] = modem;
	this->modules["modem_a"  ][0] = modem_a;
	this->modules["channel"  ][0] = channel;
	this->modules["channel_a"][0] = channel_a;

	this->monitor->add_handler_measure(std::bind(&module::Codec_SISO<B,R>::reset, codec));

	if (codec->get_decoder_siso()->get_n_frames() > 1)
		throw tools::runtime_error(__FILE__, __LINE__, __func__, "The inter frame is not supported.");
}

template <typename B, typename R>
void EXIT<B,R>
::launch()
{
	// allocate and build all the communication chain to generate EXIT chart
	this->build_communication_chain();
	this->sockets_binding();

	// for each channel SNR to be simulated	
	for (ebn0 = params.snr_min; ebn0 <= params.snr_max; ebn0 += params.snr_step)
	{
		// For EXIT simulation, SNR is considered as Es/N0
		const auto bit_rate = 1.f;
		esn0  = tools::ebn0_to_esn0 (ebn0, bit_rate, params.mdm->bps);
		sigma = tools::esn0_to_sigma(esn0, params.mdm->upf);

		terminal->set_esn0(esn0);
		terminal->set_ebn0(ebn0);

		channel->set_sigma(sigma);
		modem  ->set_sigma(sigma);
		codec  ->set_sigma(sigma);

		// for each "a" standard deviation (sig_a) to be simulated
		using namespace module;
		for (sig_a = params.sig_a_min; sig_a <= params.sig_a_max; sig_a += params.sig_a_step)
		{
			terminal ->set_sig_a(sig_a      );
			channel_a->set_sigma(2.f / sig_a);
			modem_a  ->set_sigma(2.f / sig_a);

			if (sig_a == 0.f) // if sig_a = 0, La_K2 = 0
			{
				auto &mdm = *this->modem_a;
				if (params.chn->type.find("RAYLEIGH") != std::string::npos)
				{
					auto mdm_data  = (uint8_t*)(mdm[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::Y_N2].get_dataptr());
					auto mdm_bytes =            mdm[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::Y_N2].get_databytes();
					std::fill(mdm_data, mdm_data + mdm_bytes, 0);
				}
				else
				{
					auto mdm_data  = (uint8_t*)(mdm[mdm::tsk::demodulate][mdm::sck::demodulate::Y_N2].get_dataptr());
					auto mdm_bytes =            mdm[mdm::tsk::demodulate][mdm::sck::demodulate::Y_N2].get_databytes();
					std::fill(mdm_data, mdm_data + mdm_bytes, 0);
				}
			}

			if (((!params.ter->disabled && ebn0 == params.snr_min && sig_a == params.sig_a_min  && !params.debug) ||
			    (params.statistics && !params.debug)))
				terminal->legend(std::cout);

			// start the terminal to display BER/FER results
			if (!params.ter->disabled && params.ter->frequency != std::chrono::nanoseconds(0) && !params.debug)
				this->terminal->start_temp_report(params.ter->frequency);

			this->simulation_loop();

			if (!params.ter->disabled)
			{
				if (params.debug)
					terminal->legend(std::cout);

				terminal->final_report(std::cout);

				if (params.statistics)
				{
					std::vector<std::vector<const module::Module*>> mod_vec;
					for (auto &vm : modules)
					{
						std::vector<const module::Module*> sub_mod_vec;
						for (auto *m : vm.second)
							sub_mod_vec.push_back(m);
						mod_vec.push_back(sub_mod_vec);
					}

					std::cout << "#" << std::endl;
					tools::Stats::show(mod_vec, true, std::cout);
					std::cout << "#" << std::endl;
				}
			}

			this->monitor->reset();
			for (auto &m : modules)
				for (auto mm : m.second)
					if (mm != nullptr)
						for (auto &t : mm->tasks)
							t->reset_stats();

			if (module::Monitor::is_over())
				break;
		}

		if (module::Monitor::is_over())
			break;
	}
}

template <typename B, typename R>
void EXIT<B,R>
::sockets_binding()
{
	auto &src = *this->source;
	auto &cdc = *this->codec;
	auto &enc = *this->codec->get_encoder();
	auto &dec = *this->codec->get_decoder_siso();
	auto &mdm = *this->modem;
	auto &mda = *this->modem_a;
	auto &chn = *this->channel;
	auto &cha = *this->channel_a;
	auto &mnt = *this->monitor;

	using namespace module;

	mnt[mnt::tsk::check_mutual_info][mnt::sck::check_mutual_info::bits](src[src::tsk::generate][src::sck::generate::U_K]);
	mda[mdm::tsk::modulate         ][mdm::sck::modulate         ::X_N1](src[src::tsk::generate][src::sck::generate::U_K]);
	enc[enc::tsk::encode           ][enc::sck::encode           ::U_K ](src[src::tsk::generate][src::sck::generate::U_K]);
	mdm[mdm::tsk::modulate         ][mdm::sck::modulate         ::X_N1](enc[enc::tsk::encode  ][enc::sck::encode  ::X_N]);

	// Rayleigh channel
	if (params.chn->type.find("RAYLEIGH") != std::string::npos)
	{
		cha[chn::tsk::add_noise_wg ][chn::sck::add_noise_wg ::X_N ](mda[mdm::tsk::modulate    ][mdm::sck::modulate    ::X_N2]);
		mda[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::H_N ](cha[chn::tsk::add_noise_wg][chn::sck::add_noise_wg::H_N ]);
		mda[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::Y_N1](cha[chn::tsk::add_noise_wg][chn::sck::add_noise_wg::Y_N ]);
	}
	else // additive channel (AWGN, USER, NO)
	{
		cha[chn::tsk::add_noise ][chn::sck::add_noise ::X_N ](mda[mdm::tsk::modulate ][mdm::sck::modulate ::X_N2]);
		mda[mdm::tsk::demodulate][mdm::sck::demodulate::Y_N1](cha[chn::tsk::add_noise][chn::sck::add_noise::Y_N ]);
	}

	// Rayleigh channel
	if (params.chn->type.find("RAYLEIGH") != std::string::npos)
	{
		mnt[mnt::tsk::check_mutual_info][mnt::sck::check_mutual_info::llrs_a](mda[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::Y_N2]);
		chn[chn::tsk::add_noise_wg     ][chn::sck::add_noise_wg     ::X_N   ](mdm[mdm::tsk::modulate     ][mdm::sck::modulate     ::X_N2]);
		mdm[mdm::tsk::demodulate_wg    ][mdm::sck::demodulate_wg    ::H_N   ](chn[chn::tsk::add_noise_wg ][chn::sck::add_noise_wg ::H_N ]);
		mdm[mdm::tsk::demodulate_wg    ][mdm::sck::demodulate_wg    ::Y_N1  ](chn[chn::tsk::add_noise_wg ][chn::sck::add_noise_wg ::Y_N ]);
		cdc[cdc::tsk::add_sys_ext      ][cdc::sck::add_sys_ext      ::ext   ](mda[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::Y_N2]);
		cdc[cdc::tsk::add_sys_ext      ][cdc::sck::add_sys_ext      ::Y_N   ](mdm[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::Y_N2]);
		dec[dec::tsk::decode_siso      ][dec::sck::decode_siso      ::Y_N1  ](mdm[mdm::tsk::demodulate_wg][mdm::sck::demodulate_wg::Y_N2]);
	}
	else // additive channel (AWGN, USER, NO)
	{
		mnt[mnt::tsk::check_mutual_info][mnt::sck::check_mutual_info::llrs_a](mda[mdm::tsk::demodulate][mdm::sck::demodulate::Y_N2]);
		chn[chn::tsk::add_noise        ][chn::sck::add_noise        ::X_N   ](mdm[mdm::tsk::modulate  ][mdm::sck::modulate  ::X_N2]);
		mdm[mdm::tsk::demodulate       ][mdm::sck::demodulate       ::Y_N1  ](chn[chn::tsk::add_noise ][chn::sck::add_noise ::Y_N ]);
		cdc[cdc::tsk::add_sys_ext      ][cdc::sck::add_sys_ext      ::ext   ](mda[mdm::tsk::demodulate][mdm::sck::demodulate::Y_N2]);
		cdc[cdc::tsk::add_sys_ext      ][cdc::sck::add_sys_ext      ::Y_N   ](mdm[mdm::tsk::demodulate][mdm::sck::demodulate::Y_N2]);
		dec[dec::tsk::decode_siso      ][dec::sck::decode_siso      ::Y_N1  ](mdm[mdm::tsk::demodulate][mdm::sck::demodulate::Y_N2]);
	}

	cdc[cdc::tsk::extract_sys_llr  ][cdc::sck::extract_sys_llr  ::Y_N   ](dec[dec::tsk::decode_siso    ][dec::sck::decode_siso    ::Y_N2]);
	mnt[mnt::tsk::check_mutual_info][mnt::sck::check_mutual_info::llrs_e](cdc[cdc::tsk::extract_sys_llr][cdc::sck::extract_sys_llr::Y_K ]);
}

template <typename B, typename R>
void EXIT<B,R>
::simulation_loop()
{
	auto &source    = *this->source;
	auto &codec     = *this->codec;
	auto &encoder   = *this->codec->get_encoder();
	auto &decoder   = *this->codec->get_decoder_siso();
	auto &modem     = *this->modem;
	auto &modem_a   = *this->modem_a;
	auto &channel   = *this->channel;
	auto &channel_a = *this->channel_a;
	auto &monitor   = *this->monitor;

	using namespace module;

	while (!monitor.n_trials_achieved())
	{
		if (this->params.debug)
		{
			if (!monitor[mnt::tsk::check_mutual_info].get_n_calls())
				std::cout << "#" << std::endl;

			std::cout << "# -------------------------------" << std::endl;
			std::cout << "# New communication (n°" << monitor[mnt::tsk::check_mutual_info].get_n_calls() << ")" << std::endl;
			std::cout << "# -------------------------------" << std::endl;
			std::cout << "#" << std::endl;
		}

		source [src::tsk::generate].exec();
		modem_a[mdm::tsk::modulate].exec();
		encoder[enc::tsk::encode  ].exec();
		modem  [mdm::tsk::modulate].exec();

		//if sig_a = 0, La_K = 0, no noise to add
		if (sig_a != 0)
		{
			// Rayleigh channel
			if (params.chn->type.find("RAYLEIGH") != std::string::npos)
			{
				channel_a[chn::tsk::add_noise_wg ].exec();
				modem_a  [mdm::tsk::demodulate_wg].exec();
			}
			else // additive channel (AWGN, USER, NO)
			{
				channel_a[chn::tsk::add_noise ].exec();
				modem_a  [mdm::tsk::demodulate].exec();
			}
		}

		// Rayleigh channel
		if (params.chn->type.find("RAYLEIGH") != std::string::npos)
		{
			channel[chn::tsk::add_noise_wg ].exec();
			modem  [mdm::tsk::demodulate_wg].exec();
		}
		else // additive channel (AWGN, USER, NO)
		{
			channel[chn::tsk::add_noise ].exec();
			modem  [mdm::tsk::demodulate].exec();
		}

		codec  [cdc::tsk::add_sys_ext      ].exec();
		decoder[dec::tsk::decode_siso      ].exec();
		codec  [cdc::tsk::extract_sys_llr  ].exec();
		monitor[mnt::tsk::check_mutual_info].exec();
	}
}

template <typename B, typename R>
void EXIT<B,R>
::release_objects()
{
	if (source    != nullptr) { delete source;    source    = nullptr; }
	if (codec     != nullptr) { delete codec;     codec     = nullptr; }
	if (modem     != nullptr) { delete modem;     modem     = nullptr; }
	if (modem_a   != nullptr) { delete modem_a;   modem_a   = nullptr; }
	if (channel   != nullptr) { delete channel;   channel   = nullptr; }
	if (channel_a != nullptr) { delete channel_a; channel_a = nullptr; }
	if (terminal  != nullptr) { delete terminal;  terminal  = nullptr; }
}

template <typename B, typename R>
module::Source<B>* EXIT<B,R>
::build_source()
{
	return params.src->template build<B>();
}

template <typename B, typename R>
module::Codec_SISO<B,R>* EXIT<B,R>
::build_codec()
{
	return params.cdc->template build<B,R>();
}

template <typename B, typename R>
module::Modem<B,R,R>* EXIT<B,R>
::build_modem()
{
	return params.mdm->template build<B,R>();
}

template <typename B, typename R>
module::Modem<B,R>* EXIT<B,R>
::build_modem_a()
{
	auto mdm_params = params.mdm->clone();
	mdm_params->N   = params.cdc->K;
	auto m = mdm_params->template build<B,R>();
	delete mdm_params;
	return m;
}

template <typename B, typename R>
module::Channel<R>* EXIT<B,R>
::build_channel(const int size)
{
	return params.chn->template build<R>();
}

template <typename B, typename R>
module::Channel<R>* EXIT<B,R>
::build_channel_a(const int size)
{
	auto chn_params = params.chn->clone();
	chn_params->N   = factory::Modem::get_buffer_size_after_modulation(params.mdm->type,
	                                                                   params.cdc->K,
	                                                                   params.mdm->bps,
	                                                                   params.mdm->upf,
	                                                                   params.mdm->cpm_L);

	auto c = chn_params->template build<R>();
	delete chn_params;
	return c;
}

template <typename B, typename R>
module::Monitor_EXIT<B,R>* EXIT<B,R>
::build_monitor()
{
	return params.mnt->template build<B,R>();
}

template <typename B, typename R>
tools::Terminal_EXIT<B,R>* EXIT<B,R>
::build_terminal()
{
	return params.ter->template build<B,R>(*this->monitor);
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef MULTI_PREC
template class aff3ct::simulation::EXIT<B_32,R_32>;
template class aff3ct::simulation::EXIT<B_64,R_64>;
#else
template class aff3ct::simulation::EXIT<B,R>;
#endif
// ==================================================================================== explicit template instantiation
