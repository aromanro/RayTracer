#pragma once

#include "Composite.h"

#include "ProbabilityDistributionFunction.h"

#include "Rectangle.h"


class Scene : public Objects::VisibleObjectComposite
{


public:
	Scene() : blackSky(false) {}

	inline Color BackgroundColor(const Ray& ray, Random& random)
	{
		if (skyBox)
		{
			PointInfo info;
			if (skyBox->Hit(ray, info, 1E-5, DBL_MAX, 1, random))
				return info.material->Emitted(info);	
		}
		else if (skySphere)
		{
			PointInfo info;
			if (skySphere->Hit(ray, info, 1E-5, DBL_MAX, 1, random))
				return info.material->Emitted(info);
		}
		else if (blackSky) return Color(0, 0, 0); // black sky
		
		const Vector3D<double>& dir = ray.getDirection();
		const double p = 0.5 * (dir.Y + 1);

		return (1. - p) * Color(1, 1, 1) + p * Color(0.5, 0.7, 1);		
	}

	Color RayCast(const Ray& ray, Random& random, int rcount = 0, double maxr = 300000)
	{
		if (++rcount > recursivityStop) return Color(0, 0, 0);

		PointInfo info;

		if (Hit(ray, info, 1E-5, maxr, rcount, random))
		{
			if (info.material)
			{
				const Color c = info.material->Emitted(info);				
				if (c.Emitter()) // it's a light
				{
					if (info.normal * ray.getDirection() < 0) return c;

					return Color(0, 0, 0); // no emission from the back!
				}
				else
				{					
					if (c.VeryAbsorbing()) return c; // no scatter, avoid recursion for very dark 
					maxr -= info.distance;
					if (maxr <= 0) return c; // or too far

					Materials::ScatterInfo scatterInfo;
					if (info.material->Scatter(ray, info, scatterInfo, random))
					{						
						if (scatterInfo.isSpecular) return scatterInfo.atten * RayCast(scatterInfo.specularRay, random, rcount, maxr);
							
						Ray scatteredRay;
						double PDF;

						if (PriorityObjects.size())
						{
							PDFs::VisibleObjectPDF pdfFavour(info.position, this);
							PDFs::MixturePDF pdf(scatterInfo.pdf, &pdfFavour);

							do
							{
								scatteredRay = Ray(info.position, pdf.Generate(random, &scatterInfo));
								PDF = pdf.Value(scatteredRay.getDirection(), random);
							} while (PDF < 1E-13); // see: https://github.com/petershirley/raytracingtherestofyourlife/issues/6 for explanation of this
						}
						else
						{
							//do
							//{
								scatteredRay = Ray(info.position, scatterInfo.pdf->Generate(random, &scatterInfo));
								PDF = scatterInfo.pdf->Value(scatteredRay.getDirection(), random);
							//} while (PDF < 1E-13); // see above the reason this might be needed - not needed for the current implementation, but might be needed if you set phong as not specular for small exponents, but that requires more changes in its implementation
						}
																
						return scatterInfo.atten * info.material->ScatteringPDF(ray, scatteredRay, info) * RayCast(scatteredRay, random, rcount, maxr) / PDF;
					}
				}
			}

			return Color(0, 0, 0); // no scatter
		}
	
		return BackgroundColor(ray, random); // no hit, return background color
	}


	// this gives the same weight (importance) for all objects, maybe the weight could be different depending on the some factors 
	// area? some other things that can indicate a relative importance among them?
	// as a workaround, one can add the same object twice (or more) to rise its importance relative to others (and to the whole scene)
	virtual double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
	{		
		double sum = 0;

		for (const auto& obj : PriorityObjects)
			sum += obj->pdfValue(o, v, rnd);

		return sum * invPriSize;
	}

	virtual Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
	{
		const unsigned int obj = static_cast<unsigned int>(rnd.getZeroOne() * static_cast<double>(PriorityObjects.size()));

		return PriorityObjects[obj]->getRandom(origin, rnd);
	}


	std::shared_ptr<Objects::SkyBox> skyBox;
	std::shared_ptr<Objects::SkySphere> skySphere;

	bool blackSky;

	int recursivityStop = 60;

	void AddPriorityObject(std::shared_ptr<VisibleObject> obj)
	{
		if (obj->IsComposite()) return; // composite objects need more work

		PriorityObjects.emplace_back(obj);
		invPriSize = 1. / PriorityObjects.size();
	}

	void CleanPriorityObjects()
	{
		PriorityObjects.clear();
		invPriSize = 0;
	}

protected:
	// lights and stuff like that
	std::vector<std::shared_ptr<VisibleObject>> PriorityObjects; 
	double invPriSize = 0;
};