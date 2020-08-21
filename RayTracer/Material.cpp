#include "Material.h"
#include "Random.h"
#include "OrthoNormalBasis.h"

namespace Materials
{
	const double epsilon = 1E-5;

	Material::Material(const std::shared_ptr<Textures::Texture>& texture)
		: albedo(texture)
	{		
	}


	Material::~Material()
	{
	}


	bool Lambertian::Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random)
	{
		scatterInfo.isSpecular = false;
		scatterInfo.atten = albedo->Value(info.u, info.v, info.position);

		scatterInfo.pdf = new PDFs::CosinePDF(info.normal);

		return true;
	}


	bool Metal::Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random)
	{
		const Vector3D<double> normal = info.normal;
		Vector3D<double> reflected = Reflect(incidentRay.getDirection(), normal);

		if (fuzzy) reflected += fuzzy * random.getRandomInUnitSphere();

		scatterInfo.isSpecular = true;
		scatterInfo.specularRay = Ray(info.position + epsilon * normal, reflected);
		scatterInfo.atten = albedo->Value(info.u, info.v, info.position);
		
		return info.normal * reflected > 0;
	}



	bool Dielectric::Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random)
	{
		scatterInfo.isSpecular = true;
		scatterInfo.atten = albedo->Value(info.u, info.v, info.position);

		double relativeRefrIndex;
		Vector3D<double> outNormal;
		double cosine = incidentRay.getDirection() * info.normal;

		if (cosine > 0)
		{
			// from inside the object
			outNormal = -info.normal;
			relativeRefrIndex = refrIndex;
			cosine = sqrt(1 - refrIndex * refrIndex * (1 - cosine * cosine));

			// since it was inside the object, compute the attenuation using Beer-Lambert law

			const Color absorb = info.distance * density * volumeColor;
			const Color transparency = Color(exp(-absorb.r), exp(-absorb.g), exp(-absorb.b));
			scatterInfo.atten *= transparency;
		}
		else
		{
			// from outside the object
			outNormal = info.normal;
			relativeRefrIndex = invRefrIndex;
			cosine *= -1;
		}

		Vector3D<double> refracted;
		double reflectProbability;
		if (Refract(incidentRay.getDirection(), outNormal, relativeRefrIndex, refracted))
			reflectProbability = Schlick(cosine); // Fresnel reflectance
		else
			reflectProbability = 1.; // total reflexion		

		if (random.getZeroOne() < reflectProbability)
		{
			Vector3D<double> reflected = Reflect(incidentRay.getDirection(), outNormal);
			if (fuzzy) reflected += fuzzy * random.getRandomInUnitSphere();

			scatterInfo.specularRay = Ray(info.position + epsilon * outNormal, reflected);
		}
		else
		{
			if (fuzzy) refracted += fuzzy * random.getRandomInUnitSphere();
			scatterInfo.specularRay = Ray(info.position - epsilon * outNormal, refracted);
		}

		return true;
	}


	bool Isotropic::Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random)
	{
		scatterInfo.isSpecular = false;
		scatterInfo.pdf = new PDFs::SphericalPDF();
		scatterInfo.atten = albedo->Value(info.u, info.v, info.position);
		
		return true;
	}



	bool AnisotropicPhong::Scatter(const Ray& incidentRay, const PointInfo& info, ScatterInfo& scatterInfo, Random& random)
	{
		scatterInfo.isSpecular = true;

		scatterInfo.atten = scatterInfo.diffuseColor = albedo->Value(info.u, info.v, info.position);
		scatterInfo.specularColor = specular->Value(info.u, info.v, info.position);

		scatterInfo.pdf = new PDFs::AnisotropicPhongPDF(incidentRay.getDirection(), info.normal, nu, nv);		

		if (scatterInfo.isSpecular) 
		{
			Vector3D<double> dir  = scatterInfo.pdf->Generate(random, &scatterInfo);			
			while (dir*info.normal < 0)
			{
				dir = scatterInfo.pdf->Generate(random, &scatterInfo);
			}
			scatterInfo.specularRay = Ray(info.position + epsilon * info.normal, dir);
		}

		return true;
	}



}

