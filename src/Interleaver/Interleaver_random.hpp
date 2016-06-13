#ifndef INTERLEAVER_RANDOM_HPP
#define	INTERLEAVER_RANDOM_HPP

#include <algorithm>
#include <time.h>
#include <random>

#include "Interleaver.hpp"

template <typename T>
class Interleaver_random : public Interleaver<T>
{
public:
	Interleaver_random(int size) : Interleaver<T>(size) { gen_lookup_tables(); }

protected:
	void gen_lookup_tables()
	{
		for (auto i = 0; i < (int)this->pi.size(); i++)
			this->pi[i] = i;

		std::random_device rd;
		std::mt19937 g(rd());

		std::shuffle(this->pi.begin(), this->pi.end(), g);

		for (auto i = 0; i < (int)this->pi_inv.size(); i++)
			this->pi_inv[this->pi[i]] = i;
	}
};

#endif	/* INTERLEAVER_RANDOM_HPP */
