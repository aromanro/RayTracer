#pragma once

#include <random>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Vector3D.h"

#include <limits>

class Random
{
public:
	Random(int addseed = 0);

	inline double getZeroOne()
	{
		
		return uniformZeroOne(rng);
	}

	inline Vector3D<double> getRandomInUnitSphere()
	{
		Vector3D<double> res;

		do
		{
			res = 2. * Vector3D<double>(getZeroOne(), getZeroOne(), getZeroOne()) - Vector3D<double>(1, 1, 1);
		} while (res * res >= 1);

		return res;
	}

	inline Vector3D<double> getRandomOnUnitSphere()
	{
		return getRandomInUnitSphere().Normalize();
	}

	inline Vector3D<double> getRandomCosineDirection()
	{
		const double r1 = getZeroOne();
		const double r2 = getZeroOne();
		const double z = sqrt(1. - r2);
		const double phi = 2. * M_PI * r1;
		const double twosqrt = 2. * sqrt(r2);
		const double x = cos(phi) * twosqrt;
		const double y = sin(phi) * twosqrt; 

		return Vector3D<double>(x, y, z);
	}


protected:
	std::mt19937_64 rng;
	std::uniform_real_distribution<double> uniformZeroOne;	
};

