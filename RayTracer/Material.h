#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "VisibleObject.h"
#include "Ray.h"
#include "Texture.h"
#include "Random.h"
#include "ProbabilityDistributionFunction.h"


namespace PDFs
{
	class ProbabilityDistributionFunction;
}


namespace Materials {

	struct ScatterInfo
	{
		~ScatterInfo() { delete pdf; }

		bool isSpecular = false;
		Ray specularRay;
		Color atten;
		
		Color diffuseColor;  // used only by AnisotropicPhong
		Color specularColor; // used only by AnisotropicPhong
		
		PDFs::ProbabilityDistributionFunction* pdf = nullptr;
	};


	class Material
	{
	public:
		explicit Material(const std::shared_ptr<Textures::Texture>& texture = nullptr);
		virtual ~Material() = default;

		virtual bool Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random) = 0;
		virtual double ScatteringPDF(const Ray& incidentRay, Ray& scatteredRay, const PointInfo& info) const
		{
			return 1.;
		}

		virtual Color Emitted(const PointInfo& info) const
		{
			return /*albedo ?*/ albedo->Value(info.u, info.v, info.position) /*: Color(1., 1., 1.)*/;
		}

		std::shared_ptr<Textures::Texture> albedo; // all have albedo, can absorb, even if in some cases it's set to 1
	};


	class Lambertian : public Material
	{
	public:
		using Material::Material;
		
		bool Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random) override;

		double ScatteringPDF(const Ray& incidentRay, Ray& scatteredRay, const PointInfo& info) const override
		{
			const double cosine = info.normal * scatteredRay.getDirection();

			if (cosine < 0) return 0;

			return cosine * M_1_PI;
		}
	};


	class ReflectiveMaterial : public Material
	{
	protected:
		ReflectiveMaterial() : fuzzy(0) { albedo = std::make_shared<Textures::ColorTexture>(Color(1, 1, 1)); }
		ReflectiveMaterial(const std::shared_ptr<Textures::Texture>& texture = nullptr, double f = 0) : Material(texture), fuzzy(f) { if (!albedo) albedo = std::make_shared<Textures::ColorTexture>(Color(1, 1, 1)); }

		static inline Vector3D<double> Reflect(const Vector3D<double>& incidentRay, const Vector3D<double>& normal)
		{
			const double projectionOnNormal = incidentRay * normal;

			return incidentRay - 2. * projectionOnNormal * normal;
		}
	public:
		double fuzzy;
	};


	class Metal : public ReflectiveMaterial
	{
	public:
		Metal(const std::shared_ptr<Textures::Texture>& texture = nullptr, double f = 0) : ReflectiveMaterial(texture, f) {}

		bool Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random) override;
	};


	class Dielectric : public ReflectiveMaterial
	{
	public:
		Dielectric(const double ri = 1.5, const std::shared_ptr<Textures::Texture>& texture = nullptr, double f = 0) : ReflectiveMaterial(texture, f), refrIndex(ri), invRefrIndex(1. / ri)
		{
			F0 = (1. - refrIndex) / (1. + refrIndex);
			F0 *= F0;
		}

		bool Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random) override;

		double getDensity() const { return density; }
		void setDensity(double d) { density = d; }

		Color volumeColor;

	private:
		double density = 0;
		double refrIndex;
		double invRefrIndex;
		// Fresnel reflectance
		// F0 is reflectance for normal incidence
		double F0;


		static inline bool Refract(const Vector3D<double>& incidentRay, const Vector3D<double>& normal, double relativeRefrIndex, Vector3D<double>& refractedRay)
		{
			const double projectionOnNormal = incidentRay * normal;

			if (const double cos2t = 1. - relativeRefrIndex * relativeRefrIndex * (1. - projectionOnNormal * projectionOnNormal); cos2t > 0)
			{
				refractedRay = relativeRefrIndex * (incidentRay - projectionOnNormal * normal) - sqrt(cos2t) * normal;
				return true;
			}

			return false; // total internal reflection!
		}


		inline double Schlick(double cosine) const
		{
			// for an angle:
			return F0 + (1. - F0) * pow(1. - cosine, 5.);
		}
	};

	class Isotropic : public Material
	{
	public:
		using Material::Material;

		bool Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random) override;

		double ScatteringPDF(const Ray& incidentRay, Ray& scatteredRay, const PointInfo& info) const override
		{
			return 0.25 * M_1_PI;
		}
	};


	class AnisotropicPhong : public Material
	{
	public:
		AnisotropicPhong(double Nu, double Nv, const std::shared_ptr<Textures::Texture>& diffuseTexture = nullptr, const std::shared_ptr<Textures::Texture>& specularTexture = nullptr, const std::shared_ptr<Textures::Texture>& exponentTexture = nullptr) 
			: Material(diffuseTexture), specular(specularTexture), exponent(exponentTexture), nu(Nu), nv(Nv) {}

		bool Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random) override;

		double ScatteringPDF(const Ray& incidentRay, Ray& scatteredRay, const PointInfo& info) const override
		{
			const double cosine = info.normal * scatteredRay.getDirection();

			if (cosine < 0) return 0;

			return cosine * M_1_PI;
		}


	private:
		std::shared_ptr<Textures::Texture> specular;
		std::shared_ptr<Textures::Texture> exponent;

		// exponents - for nu=nv it's similar with Phong 
		double nu;
		double nv;
	};

}

