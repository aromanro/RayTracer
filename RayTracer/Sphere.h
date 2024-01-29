#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "VisibleObject.h"
#include "OrthoNormalBasis.h"
#include "Material.h"
#include "ImageTexture.h"

namespace Materials {
	class Material;
}

namespace Objects
{

	class Sphere : public VisibleObjectElementary
	{
	public:
		Sphere(const Vector3D<double>& position, double R, const std::shared_ptr<Materials::Material>& m);

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override;

		
		void ConstructBoundingBox() override
		{
			const Vector3D<double> c = Vector3D<double>(radius, radius, radius);
			boundingBox = BVH::AxisAlignedBoundingBox(center - c, center + c);
		}

		const Vector3D<double>& getCenter() const { return center; }
		double getRadius() const { return radius; }

		void getUV(const Vector3D<double>& pos, double& u, double& v) const;


		void Translate(const Vector3D<double>& t) override
		{
			center += t;
			boundingBox.Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			center = center.RotateAround(v, angle);

			// this method has a big issue, it doesn't work if rotation is done around Ox
			// I will rotate them probably only around Oz, so I don't care for something better

			static const double TWO_M_PI = 2. * M_PI;
			const Vector3D<double> X(1, 0, 0);
			const Vector3D<double> Xn = X.RotateAround(v, angle);

			startTheta += asin(Xn.Y);
			startPhi += atan2(Xn.Z, Xn.X);

			if (startTheta > M_PI_2) startTheta -= M_PI;
			else if (startTheta < -M_PI_2) startTheta += M_PI;

			if (startPhi > M_PI) startPhi -= TWO_M_PI;
			else if (startPhi < -M_PI) startPhi += TWO_M_PI;

			ConstructBoundingBox();
		}

		void Scale(double s) override
		{
			center *= s;
			radius *= s;

			boundingBox.Scale(s);
		}

		double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override 
		{ 
			if (PointInfo info; Hit(Ray(o, v), info, 1E-5, DBL_MAX, 1, rnd))
			{
				static const double TWO_M_PI = 2. * M_PI;

				const Vector3D<double> d = center - o;
				const double CosThetaMax = sqrt(1. - R2 / (d * d));
				const double solidAngle = TWO_M_PI * (1. - CosThetaMax);

				return 1. / solidAngle;
			}

			return 0; 
		}
		
		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override 
		{ 
			const Vector3D<double> d = center - origin;
			const double d2 = d * d;

			OrthoNormalBasis onb(d);

			return onb.LocalToGlobal(RandomToSphere(radius, d2, rnd));
		}

		virtual Vector3D<double> getNormal(const PointInfo& info) const
		{
			return (info.position - center).Normalize();
		}

	private:
		static inline Vector3D<double> RandomToSphere(double Radius, double dist2, Random& rnd)
		{
			static const double TWO_M_PI = 2. * M_PI;
			const double r1 = rnd.getZeroOne();
			const double r2 = rnd.getZeroOne();
			const double z = 1. + r2 * (sqrt(1. - Radius * Radius / dist2) - 1.);
			const double sonemz2 = sqrt(1. - z * z);
			const double Phi = TWO_M_PI * r1;

			const double x = cos(Phi) * sonemz2;
			const double y = sin(Phi) * sonemz2;

			return Vector3D<double>(x, y, z);
		}


		Vector3D<double> center;
		double radius;
		double R2;

		double startTheta;
		double startPhi;
	};


	class InvertedSphere : public Sphere
	{
	public:
		using Sphere::Sphere;

		Vector3D<double> getNormal(const PointInfo& info) const override
		{
			return -Sphere::getNormal(info);
		}
	};




	// TODO: not tested yet, probably does not work!
	// use ConstantMedium for it
	// the 'inside' will be actually outside, so there will be no Beer-Lambert
	// but it can be implemented if needed

	class SkySphere : public InvertedSphere
	{
	public:
		SkySphere(const Vector3D<double>& position, double R)
			: InvertedSphere(position, R, std::make_shared<Materials::Lambertian>())
		{
		}

		bool Load(const std::string& texName)
		{
			try
			{
				auto tf = std::make_shared<Textures::ImageTexture>(texName);

				SetMaterial(std::make_shared<Materials::Lambertian>(tf));
			}
			catch (...)
			{
				return false;
			}

			return true;
		}
	};
}