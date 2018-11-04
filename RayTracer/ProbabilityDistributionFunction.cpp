#include "ProbabilityDistributionFunction.h"
#include "Material.h"

namespace PDFs
{

	Vector3D<double> AnisotropicPhongPDF::Generate(Random& rnd, Materials::ScatterInfo* info)
	{
		double xi = rnd.getZeroOne();
		
		double phase = 0;
		bool flip = false;


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

		const double kh = -incident * h;
		const double specularProbability = GetSpecularPDH(h, kh, cos2, sin2);
		const double weight = 1. + specularProbability;

		const double diffuseProbability = 1. / weight;


		if (rnd.getZeroOne() < diffuseProbability)
		{
			info->atten = info->diffuseColor;
			return onb.LocalToGlobal(rnd.getRandomCosineDirection());
		}

		info->atten = info->specularColor;		
		return GetSpecularReflected(incident, h, kh);
	}


}