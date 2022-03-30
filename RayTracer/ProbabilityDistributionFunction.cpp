#include "ProbabilityDistributionFunction.h"
#include "Material.h"

namespace PDFs
{

	void AnisotropicPhongPDF::DealWithQuadrants(double& xi, double& phase, bool& flip)
	{
		phase = 0;
		flip = false;

		if (xi < 0.25)
		{
			xi *= 4;
		}
		else if (xi < 0.5)
		{
			xi = 1. - 4. * (0.5 - xi);
			phase = M_PI;
			flip = true;
		}
		else if (xi < 0.75)
		{
			xi = 1. - 4. * (0.75 - xi);
			phase = M_PI;
		}
		else
		{
			xi = 1. - 4. * (1. - xi);
			phase = 2. * M_PI;
			flip = true;
		}
	}


	Vector3D<double> AnisotropicPhongPDF::Generate(Random& rnd, Materials::ScatterInfo* info)
	{
		double phase;
		bool flip;

		double xi = rnd.getZeroOne();		
		DealWithQuadrants(xi, phase, flip);

		double phi = atan(prefactor1 * tan(M_PI_2 * xi));
		if (flip)
			phi = phase - phi;
		else
			phi += phase;

		const double c = cos(phi);
		const double s = sin(phi);
		const double c2 = c * c;
		const double s2 = 1. - c2;
		
		xi = rnd.getZeroOne();
		DealWithQuadrants(xi, phase, flip);
			
		double theta = acos(pow(1. - xi, 1. / (nu * c2 + nv * s2 + 1.)));				
		if (flip)
			theta = phase - theta;
		else
			theta += phase;
				
		const double st = sin(theta);
		const double ct = cos(theta);

		const double cos2 = ct * ct;
		const double sin2 = st * st;

		const Vector3D<double> h = onb.LocalToGlobal(Vector3D<double>(st * c, st * s, ct));

		double diffuseProbability;
		double kh = 0.; // avoid complains about not being initialized

		if (h * onb.Normal() < 0)
			diffuseProbability = 1.;
		else
		{
			kh = -incident * h;
			const double specularProbability = GetSpecularPDH(h, kh, cos2, sin2);
			const double weight = 1. + specularProbability;

			diffuseProbability = 1. / weight;
		}

		if (rnd.getZeroOne() < diffuseProbability)
		{
			info->atten = info->diffuseColor;
			return onb.LocalToGlobal(rnd.getRandomCosineDirection());
		}

		info->atten = 0.8 * info->specularColor + 0.2 * info->diffuseColor; // I don't like the white specular color that's typical in obj files, mix it with the diffuse color
		return GetSpecularReflected(incident, h, kh);
	}


}