#pragma once

#include "Vector3D.h"
#include "OrthoNormalBasis.h"
#include "Random.h"
#include "VisibleObject.h"
#include "Random.h"

namespace Materials {
	struct ScatterInfo;
}


namespace PDFs
{

	class ProbabilityDistributionFunction
	{
	public:
		virtual ~ProbabilityDistributionFunction() = default;

		virtual double Value(const Vector3D<double>& dir, Random& rnd) = 0;
		virtual Vector3D<double> Generate(Random& rnd, Materials::ScatterInfo* info) = 0;
	};



	class CosinePDF : public ProbabilityDistributionFunction
	{
	public:
		explicit CosinePDF(const Vector3D<double>& dir)
			: onb(dir)
		{
		}

		double Value(const Vector3D<double>& dir, Random& rnd) override
		{
			const double cosine = dir * onb.w();

			if (cosine < 0) return 0;

			return cosine * M_1_PI;
		}		

		Vector3D<double> Generate(Random& rnd, Materials::ScatterInfo* info) override
		{
			return onb.LocalToGlobal(rnd.getRandomCosineDirection());
		}

	private:
		OrthoNormalBasis onb;
	};


	class SphericalPDF : public ProbabilityDistributionFunction
	{
	public:
		double Value(const Vector3D<double>& dir, Random& rnd) override
		{
			return 0.25 * M_1_PI;
		}

		Vector3D<double> Generate(Random& rnd, Materials::ScatterInfo* info) override
		{
			return rnd.getRandomInUnitSphere();
		}
	};


	class VisibleObjectPDF : public ProbabilityDistributionFunction
	{
	public:
		VisibleObjectPDF(const Vector3D<double>& o, const Objects::VisibleObject* obj)
			: origin(o), object(obj)
		{
		}

		double Value(const Vector3D<double>& dir, Random& rnd) override
		{
			return object->pdfValue(origin, dir, rnd);
		}
		

		Vector3D<double> Generate(Random& rnd, Materials::ScatterInfo* info) override
		{
			return object->getRandom(origin, rnd); // generates a random ray pointing to the object, using a uniform distribution over the surface
		}

	private:
		const Vector3D<double> origin;
		const Objects::VisibleObject* object;
	};


	class MixturePDF : public ProbabilityDistributionFunction
	{
	public:
		MixturePDF(ProbabilityDistributionFunction* pdf1, ProbabilityDistributionFunction* pdf2)
			: m_pdf1(pdf1), m_pdf2(pdf2)
		{
		}

		double Value(const Vector3D<double>& dir, Random& rnd) override
		{
			return 0.5 * m_pdf1->Value(dir, rnd) + 0.5 * m_pdf2->Value(dir, rnd);
		}

		Vector3D<double> Generate(Random& rnd, Materials::ScatterInfo* info) override
		{
			if (rnd.getZeroOne() < 0.5) return m_pdf1->Generate(rnd, info);

			return m_pdf2->Generate(rnd, info);
		}

	private:
		ProbabilityDistributionFunction* m_pdf1;
		ProbabilityDistributionFunction* m_pdf2;
	};


	// See 'An Anisotropic Phong BRDF Model' by Michael Ashikhmin and Peter Shirley
	// https://www.cs.utah.edu/~shirley/papers/jgtbrdf.pdf

	class AnisotropicPhongPDF : public ProbabilityDistributionFunction
	{
	public:
		AnisotropicPhongPDF(const Vector3D<double>& inc, const Vector3D<double>& norm, double Nu, double Nv)
			: incident(inc), onb(norm, inc), nu(Nu), nv(Nv)
		{
			const double nu1 = nu + 1.;
			const double nv1 = nv + 1.;
			prefactor1 = sqrt(nu1 / nv1);
			prefactor2 = sqrt(nu1 * nv1) / (2. * M_PI);
		}


		double Value(const Vector3D<double>& dir, Random& rnd) override
		{
			const double cosine = dir * onb.Normal();
			if (cosine < 0) return 0;

			return cosine * M_1_PI;
		}

		Vector3D<double> Generate(Random& rnd, Materials::ScatterInfo* info) override;

	private:
		inline static double Schlick(const double val, double cosine)
		{
			return val + (1. - val) * pow(1. - cosine, 5.);
		}

		inline Vector3D<double> GetSpecularReflected(const Vector3D<double>& incident, const Vector3D<double>& h, double kh) const
		{
			return incident + 2. * kh * h;
		}

		inline double GetSpecularPDH(const Vector3D<double>& h, double kh, double cos2, double sin2) const
		{
			return GetHPDH(h, cos2, sin2) / (4. * kh);
		}

		inline double GetHPDH(const Vector3D<double>& h, double cos2, double sin2) const
		{
			const double nh = h * onb.Normal();

			return prefactor2 * pow(nh, nu * cos2 + nv * sin2);
		}

		static void DealWithQuadrants(double& xi, double& phase, bool& flip);

		Vector3D<double> incident;
		OrthoNormalBasis onb;
		
		double nu;
		double nv;

		double prefactor1;
		double prefactor2;
	};

}