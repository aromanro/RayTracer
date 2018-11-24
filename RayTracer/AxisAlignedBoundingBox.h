#pragma once

#include <algorithm>

#include "Vector3D.h"
#include "Ray.h"

namespace BVH
{

	class AxisAlignedBoundingBox
	{
	protected:
		Vector3D<double> m_min;
		Vector3D<double> m_max;

		inline static double mmin(double a, double b) { return a < b ? a : b; }
		inline static double mmax(double a, double b) { return a > b ? a : b; }

		// The following are currently not used, I used them trying some things during development:

		inline static Vector3D<double> mmin(const Vector3D<double>& a, const Vector3D<double>& b)
		{
			return Vector3D<double>(mmin(a.X, b.X), mmin(a.Y, b.Y), mmin(a.Z, b.Z));
		}

		inline static Vector3D<double> mmax(const Vector3D<double>& a, const Vector3D<double>& b)
		{
			return Vector3D<double>(mmax(a.X, b.X), mmax(a.Y, b.Y), mmax(a.Z, b.Z));
		}

		inline static double maxComp(const Vector3D<double>& v)
		{
			return mmax(mmax(v.X, v.Y), v.Z);
		}

		inline static double minComp(const Vector3D<double>& v)
		{
			return mmin(mmin(v.X, v.Y), v.Z);
		}

		// until here
	public:
		AxisAlignedBoundingBox() {}
		AxisAlignedBoundingBox(const Vector3D<double>& mi, const Vector3D<double>& ma) : m_min(mi), m_max(ma) {}

		inline const Vector3D<double>& min() const { return m_min; }
		inline const Vector3D<double>& max() const { return m_max; }

		inline bool Intersects(const Ray& ray, double tmin, double tmax) const
		{			
			double t0 = (m_min.X - ray.getOrigin().X) * ray.getInvDir().X;
			double t1 = (m_max.X - ray.getOrigin().X) * ray.getInvDir().X;
			if (ray.getInvDir().X < 0) std::swap(t0, t1);

			tmin = mmax(t0, tmin);
			tmax = mmin(t1, tmax);
			if (tmax < tmin) return false;


			t0 = (m_min.Y - ray.getOrigin().Y) * ray.getInvDir().Y;
			t1 = (m_max.Y - ray.getOrigin().Y) * ray.getInvDir().Y;
			if (ray.getInvDir().Y < 0) std::swap(t0, t1);

			tmin = mmax(t0, tmin);
			tmax = mmin(t1, tmax);
			if (tmax < tmin) return false;


			t0 = (m_min.Z - ray.getOrigin().Z) * ray.getInvDir().Z;
			t1 = (m_max.Z - ray.getOrigin().Z) * ray.getInvDir().Z;
			if (ray.getInvDir().Z < 0) std::swap(t0, t1);

			tmin = mmax(t0, tmin);
			tmax = mmin(t1, tmax);
			
			return tmax >= tmin;
		}

		void Translate(const Vector3D<double>& t)
		{
			m_min += t;
			m_max += t;
		}

		void Scale(double s)
		{
			m_min *= s;
			m_max *= s;
		}

		inline static AxisAlignedBoundingBox EnclosingBox(const AxisAlignedBoundingBox& b1, const AxisAlignedBoundingBox& b2)
		{
			return AxisAlignedBoundingBox(Vector3D<double>(mmin(b1.m_min.X, b2.m_min.X), mmin(b1.m_min.Y, b2.m_min.Y), mmin(b1.m_min.Z, b2.m_min.Z)),
				Vector3D<double>(mmax(b1.m_max.X, b2.m_max.X), mmax(b1.m_max.Y, b2.m_max.Y), mmax(b1.m_max.Z, b2.m_max.Z)));
		}
	};

}

