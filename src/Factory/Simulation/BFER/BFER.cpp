#include <thread>

#include "BFER.hpp"

using namespace aff3ct;
using namespace aff3ct::factory;

const std::string aff3ct::factory::BFER::name   = "Simulation BFER";
const std::string aff3ct::factory::BFER::prefix = "sim";

BFER::parameters
::parameters(const std::string name, const std::string prefix)
: Simulation::parameters(name, prefix)
{
}

BFER::parameters
::~parameters()
{
	if (src != nullptr) { delete src; src = nullptr; }
	if (crc != nullptr) { delete crc; crc = nullptr; }
	if (cdc != nullptr) { delete cdc; cdc = nullptr; }
	if (mdm != nullptr) { delete mdm; mdm = nullptr; }
	if (chn != nullptr) { delete chn; chn = nullptr; }
	if (qnt != nullptr) { delete qnt; qnt = nullptr; }
	if (mnt != nullptr) { delete mnt; mnt = nullptr; }
	if (ter != nullptr) { delete ter; ter = nullptr; }
}

BFER::parameters* BFER::parameters
::clone() const
{
	auto clone = new BFER::parameters(*this);

	if (src != nullptr) { clone->src = src->clone(); }
	if (crc != nullptr) { clone->crc = crc->clone(); }
	if (cdc != nullptr) { clone->cdc = cdc->clone(); }
	if (mdm != nullptr) { clone->mdm = mdm->clone(); }
	if (chn != nullptr) { clone->chn = chn->clone(); }
	if (qnt != nullptr) { clone->qnt = qnt->clone(); }
	if (mnt != nullptr) { clone->mnt = mnt->clone(); }
	if (ter != nullptr) { clone->ter = ter->clone(); }

	return clone;
}

std::vector<std::string> BFER::parameters
::get_names() const
{
	auto n = Simulation::parameters::get_names();
	if (src != nullptr) { auto nn = src->get_names(); for (auto &x : nn) n.push_back(x); }
	if (crc != nullptr) { auto nn = crc->get_names(); for (auto &x : nn) n.push_back(x); }
	if (cdc != nullptr) { auto nn = cdc->get_names(); for (auto &x : nn) n.push_back(x); }
	if (mdm != nullptr) { auto nn = mdm->get_names(); for (auto &x : nn) n.push_back(x); }
	if (chn != nullptr) { auto nn = chn->get_names(); for (auto &x : nn) n.push_back(x); }
	if (qnt != nullptr) { auto nn = qnt->get_names(); for (auto &x : nn) n.push_back(x); }
	if (mnt != nullptr) { auto nn = mnt->get_names(); for (auto &x : nn) n.push_back(x); }
	if (ter != nullptr) { auto nn = ter->get_names(); for (auto &x : nn) n.push_back(x); }
	return n;
}

std::vector<std::string> BFER::parameters
::get_short_names() const
{
	auto sn = Factory::parameters::get_short_names();
	if (src != nullptr) { auto nn = src->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	if (crc != nullptr) { auto nn = crc->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	if (cdc != nullptr) { auto nn = cdc->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	if (mdm != nullptr) { auto nn = mdm->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	if (chn != nullptr) { auto nn = chn->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	if (qnt != nullptr) { auto nn = qnt->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	if (mnt != nullptr) { auto nn = mnt->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	if (ter != nullptr) { auto nn = ter->get_short_names(); for (auto &x : nn) sn.push_back(x); }
	return sn;
}

std::vector<std::string> BFER::parameters
::get_prefixes() const
{
	auto p = Factory::parameters::get_prefixes();
	if (src != nullptr) { auto nn = src->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	if (crc != nullptr) { auto nn = crc->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	if (cdc != nullptr) { auto nn = cdc->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	if (mdm != nullptr) { auto nn = mdm->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	if (chn != nullptr) { auto nn = chn->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	if (qnt != nullptr) { auto nn = qnt->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	if (mnt != nullptr) { auto nn = mnt->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	if (ter != nullptr) { auto nn = ter->get_prefixes(); for (auto &x : nn) p.push_back(x); }
	return p;
}

void BFER::parameters
::get_description(arg_map &req_args, arg_map &opt_args) const
{
	Simulation::parameters::get_description(req_args, opt_args);

	auto p = this->get_prefix();

	opt_args[{p+"-snr-type", "E"}] =
		{"string",
		 "select the type of SNR: symbol energy or information bit energy.",
		 "ES, EB"};

	opt_args[{p+"-coset", "c"}] =
		{"",
		 "enable the coset approach."};

	opt_args[{p+"-err-trk"}] =
		{"",
		 "enable the tracking of the bad frames (by default the frames are stored in the current folder)."};

	opt_args[{p+"-err-trk-rev"}] =
		{"",
		 "automatically replay the saved frames."};

	opt_args[{p+"-err-trk-path"}] =
		{"string",
		 "base path for the files where the bad frames will be stored or read."};

	opt_args[{p+"-err-trk-thold"}] =
		{"positive_int",
		 "dump only frames with a bit error count above or equal to this threshold."};

	opt_args[{p+"-coded"}] =
		{"",
		 "enable the coded monitoring (extends the monitored bits to the entire codeword)."};
}

void BFER::parameters
::store(const arg_val_map &vals)
{
#if !defined(SYSTEMC)
	this->n_threads = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1;
#endif

	Simulation::parameters::store(vals);

	auto p = this->get_prefix();

	if(exist(vals, {p+"-snr-type",   "E"})) this->snr_type            =           vals.at({p+"-snr-type", "E"});
	if(exist(vals, {p+"-err-trk-path"   })) this->err_track_path      =           vals.at({p+"-err-trk-path" });
	if(exist(vals, {p+"-err-trk-thold"  })) this->err_track_threshold = std::stoi(vals.at({p+"-err-trk-thold"}));
	if(exist(vals, {p+"-err-trk-rev"    })) this->err_track_revert    = true;
	if(exist(vals, {p+"-err-trk"        })) this->err_track_enable    = true;
	if(exist(vals, {p+"-coset",      "c"})) this->coset               = true;
	if(exist(vals, {p+"-coded",         })) this->coded_monitoring    = true;

	if (this->err_track_revert)
	{
		this->err_track_enable = false;
		this->n_threads = 1;
	}
}

void BFER::parameters
::get_headers(std::map<std::string,header_list>& headers, const bool full) const
{
	Simulation::parameters::get_headers(headers, full);

	auto p = this->get_prefix();

	headers[p].push_back(std::make_pair("SNR type", this->snr_type));
	headers[p].push_back(std::make_pair("Coset approach (c)", this->coset ? "yes" : "no"));
	headers[p].push_back(std::make_pair("Coded monitoring", this->coded_monitoring ? "yes" : "no"));

	std::string enable_track = (this->err_track_enable) ? "on" : "off";
	headers[p].push_back(std::make_pair("Bad frames tracking", enable_track));

	std::string enable_rev_track = (this->err_track_revert) ? "on" : "off";
	headers[p].push_back(std::make_pair("Bad frames replay", enable_rev_track));

	if (this->err_track_threshold)
		headers[p].push_back(std::make_pair("Bad frames threshold", std::to_string(this->err_track_threshold)));

	if (this->err_track_enable || this->err_track_revert)
	{
		std::string path = this->err_track_path + std::string("_$snr.[src,enc,chn]");
		headers[p].push_back(std::make_pair("Bad frames base path", path));
	}

	if (this->src != nullptr && this->cdc != nullptr)
	{
		const auto bit_rate = (float)this->src->K / (float)this->cdc->N;
		headers[p].push_back(std::make_pair("Bit rate", std::to_string(bit_rate)));
	}

	if (this->src != nullptr)
		headers[p].push_back(std::make_pair("Inter frame level", std::to_string(this->src->n_frames)));

	if (this->src != nullptr) { this->src->get_headers(headers, full); }
	if (this->crc != nullptr) { this->crc->get_headers(headers, full); }
	if (this->cdc != nullptr) { this->cdc->get_headers(headers, full); }
	if (this->mdm != nullptr) { this->mdm->get_headers(headers, full); }
	if (this->chn != nullptr) { this->chn->get_headers(headers, full); }
	if (this->qnt != nullptr) { this->qnt->get_headers(headers, full); }
	if (this->mnt != nullptr) { this->mnt->get_headers(headers, full); }
	if (this->ter != nullptr) { this->ter->get_headers(headers, full); }
}
