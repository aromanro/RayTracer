
#include "Sphere.h"
#include "Random.h"

namespace Objects {

	Sphere::Sphere(const Vector3D<double>& position, double R, const std::shared_ptr<Materials::Material>& m)
		: center(position), radius(R), R2(R*R)
	{
		VisibleObjectElementary::SetMaterial(m);

		const Vector3D c(radius, radius, radius);
		boundingBox = BVH::AxisAlignedBoundingBox(center - c, center + c);
	}


	bool Sphere::Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const
	{
		const Vector3D v = ray.getOrigin() - center;

		// a = 1 because ray direction is normalized

		// in the following, the 2 was eliminated
		const double b = v * ray.getDirection();
		const double c = v * v - R2;

		if (const double d = b * b - c; d > 0)
		{
			const double sd = sqrt(d);
			double dist = -b - sd;

			if (dist > minr && dist < maxr)
			{
				info.distance = dist;
				info.position = ray(dist);
				info.normal = getNormal(info);
				info.material = GetMaterial().get();
				info.object = this;

				getUV(info.position, info.u, info.v);

				return true;
			}

			dist = -b + sd;
			if (dist > minr && dist < maxr)
			{
				info.distance = dist;
				info.position = ray(dist);
				info.normal = getNormal(info);
				info.material = GetMaterial().get();
				info.object = this;


				getUV(info.position, info.u, info.v);

				return true;
			}
		}

		return false;
	}

	void Sphere::getUV(const Vector3D<double>& pos, double& u, double& v) const
	{
		static const double TWO_M_PI = 2. * M_PI;
		static const double invTWO_M_PI = 1. / TWO_M_PI;

		const Vector3D dir = (pos - center).Normalize();

		double theta = asin(dir.Y) + startTheta;
		double phi = atan2(dir.Z, dir.X) + startPhi;

		if (theta > M_PI_2) theta -= M_PI;
		else if (theta < -M_PI_2) theta += M_PI;

		if (phi > M_PI) phi -= TWO_M_PI;
		else if (phi < -M_PI) phi += TWO_M_PI;

		u = 1. - (phi + M_PI) * invTWO_M_PI;
		v = (theta + M_PI_2) * M_1_PI;
	}

}