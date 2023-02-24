#include "Random.h"

#include <chrono>


Random::Random(int addseed)
	: uniformZeroOne(0, 1), gaussian(0.0, 1.0)
{
	uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	timeSeed += addseed;
	std::seed_seq seed{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
		
	rng.seed(seed);
}

