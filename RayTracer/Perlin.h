#pragma once

#include <numeric>

#include "Random.h"
#include "Vector3D.h"
#include "Color.h"

#include "Texture.h"



namespace Textures
{
	class NoiseBase
	{
	public:
		virtual ~NoiseBase() = default;

		virtual double Noise(const Vector3D<double>& point) const = 0;
		virtual double Turbulence(const Vector3D<double>& p, int depth = 7) const = 0;
	};

	class Perlin : public NoiseBase
	{
	public:
		Perlin(Random& random, int pointCnt = 256)
		{
			if (pointCnt == 0) pointCnt = 256;

			ranVector.resize(pointCnt);
			for (size_t i = 0; i < pointCnt; ++i)
				ranVector[i] = random.getRandomOnUnitSphere();

			permX.resize(pointCnt);
			permY.resize(pointCnt);
			permZ.resize(pointCnt);

			std::iota(permX.begin(), permX.end(), 0);
			std::iota(permY.begin(), permY.end(), 0);
			std::iota(permZ.begin(), permZ.end(), 0);

			std::random_device rd;
			std::mt19937 g(rd());

			std::shuffle(permX.begin(), permX.end(), g);
			std::shuffle(permY.begin(), permY.end(), g);
			std::shuffle(permZ.begin(), permZ.end(), g);
		}


		double Noise(const Vector3D<double>& p) const override
		{
			int i = floor(p.X);
			int j = floor(p.Y);
			int k = floor(p.Z);

			double u = p.X - i;
			double v = p.Y - j;
			double w = p.Z - k;

			u = u * u * (3 - 2 * u);
			v = v * v * (3 - 2 * v);
			w = w * w * (3 - 2 * w);

			std::array<std::array<std::array<Vector3D<double>, 2>, 2>, 2> c;

			for (int di = 0; di < 2; ++di)
				for (int dj = 0; dj < 2; ++dj)
					for (int dk = 0; dk < 2; ++dk)
						c[di][dj][dk] = ranVector[permX[(i + di) & 255] ^ permY[(j + dj) & 255] ^ permZ[(k + dk) & 255]];

			return trilinear_interpolation(c, u, v, w);
		}


		double Turbulence(const Vector3D<double>& p, int depth = 7) const override
		{
			auto accum = 0.0;
			auto temp_p = p;
			auto weight = 1.0;

			for (int i = 0; i < depth; i++) {
				accum += weight * Noise(temp_p);
				weight *= 0.5;
				temp_p *= 2;
			}

			return fabs(accum);
		}

	private:
		inline double trilinear_interpolation(const std::array<std::array<std::array<Vector3D<double>, 2>, 2>, 2>& c, double u, double v, double w) const
		{
			auto uu = u * u * (3 - 2 * u);
			auto vv = v * v * (3 - 2 * v);
			auto ww = w * w * (3 - 2 * w);
			auto accum = 0.0;

			for (int i = 0; i < 2; ++i)
				for (int j = 0; j < 2; ++j)
					for (int k = 0; k < 2; ++k)
					{
						const Vector3D<double> weight_v(u - i, v - j, w - k);

						accum += (i * uu + (1LL - i) * (1LL - uu))
							* (j * vv + (1LL - j) * (1LL - vv))
							* (k * ww + (1LL - k) * (1LL - ww))
							* c[i][j][k] * weight_v;
					}

			return accum;
		}

		std::vector<Vector3D<double>> ranVector;
		std::vector<size_t> permX;
		std::vector<size_t> permY;
		std::vector<size_t> permZ;
	};

	
	class PerlinNoiseTexture : public ColorTexture 
	{
	public:
		PerlinNoiseTexture(Random& random, int nrPoints = 256, const Color& color = Color(1., 1., 1.), double scale = 1.)
			: ColorTexture(color), noise(random, nrPoints), m_scale(scale)
		{
		}

		Color Value(double u, double v, const Vector3D<double>& p) const override 
		{
			return GetColor() * 0.5 * (1.0 + noise.Noise(m_scale * p));
		}

		const Perlin& GetNoise() const { return noise; }
		double GetScale() const { return m_scale; }

	private:
		Perlin noise;
		double m_scale;
	};


	class PerlinTurbulenceTexture : public PerlinNoiseTexture
	{
	public:
		PerlinTurbulenceTexture(Random& random, int nrPoints = 256, const Color& color = Color(1., 1., 1.), double scale = 1.)
			: PerlinNoiseTexture(random, nrPoints, color, scale)
		{
		}

		Color Value(double u, double v, const Vector3D<double>& p) const override
		{
			return GetColor() * GetNoise().Turbulence(GetScale() * p);
		}
	};

	class PerlinMarbleTexture : public PerlinNoiseTexture
	{
	public:
		PerlinMarbleTexture(Random& random, int nrPoints = 256, const Color& color = Color(1., 1., 1.), double scale = 1.)
			: PerlinNoiseTexture(random, nrPoints, color, scale)
		{
		}

		Color Value(double u, double v, const Vector3D<double>& p) const override
		{
			return GetColor() * 0.5 * (1. + sin(GetScale() * p.Z + 10. * GetNoise().Turbulence(p)));
		}
	};

}

