#pragma once

#include "Vector3D.h"
#include "VisibleObject.h"
#include "Actions.h"

namespace Objects {

	class Triangle : public VisibleObjectElementary
	{
	public:
		Triangle() : area(0), U1(-1), V1(-1), U2(-1), V2(-1), U3(-1), V3(-1) {}
		Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const std::shared_ptr<Materials::Material>& m = nullptr);
		Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const Vector3D<double>& n1, const Vector3D<double>& n2, const Vector3D<double>& n3, const std::shared_ptr<Materials::Material>& m = nullptr);

		virtual void ConstructBoundingBox() override
		{
			Vector3D<double> minv(std::min(std::min(A.X, B.X), C.X), std::min(std::min(A.Y, B.Y), C.Y), std::min(std::min(A.Z, B.Z), C.Z));
			Vector3D<double> maxv(std::max(std::max(A.X, B.X), C.X), std::max(std::max(A.Y, B.Y), C.Y), std::max(std::max(A.Z, B.Z), C.Z));

			const Vector3D<double> dif = maxv - minv;

			if (dif.X < 1E-5) maxv.X += 1E-5;
			if (dif.Y < 1E-5) maxv.Y += 1E-5;
			if (dif.Z < 1E-5) maxv.Z += 1E-5;

			boundingBox = BVH::AxisAlignedBoundingBox(minv, maxv);
		}

		virtual void Translate(const Vector3D<double>& t) override
		{
			A += t;
			B += t;
			C += t;

			boundingBox.Translate(t);
		}

		virtual void RotateAround(const Vector3D<double>& v, double angle) override
		{
			A = A.RotateAround(v, angle);
			B = B.RotateAround(v, angle);
			C = C.RotateAround(v, angle);
			
			edge1 = B - A;
			edge2 = C - A;
			// area does not change

			normal = (edge1 % edge2).Normalize();

			if (threeNormals)
			{
				normal1 = normal1.RotateAround(v, angle);
				normal2 = normal2.RotateAround(v, angle);
				normal3 = normal3.RotateAround(v, angle);
			}

			ConstructBoundingBox();
		}

		virtual void Scale(double scale) override
		{
			A *= scale;
			B *= scale;
			C *= scale;
			edge1 *= scale;
			edge2 *= scale;
			
			area *= scale * scale;

			boundingBox.Scale(scale);
		}


		virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override;


		inline static double Interpolate(const PointInfo& info, double val1, double val2, double val3)
		{
			const double w = 1. - info.u - info.v;
			return w * val1 + info.u * val2 + info.v * val3;
		}

		inline static Color Interpolate(const PointInfo& info, const Color& col1, const Color& col2, const Color& col3)
		{
			const double w = 1. - info.u - info.v;
			return w * col1 + info.u * col2 + info.v * col3;
		}

		virtual double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override 
		{ 
			PointInfo info;

			if (Hit(Ray(o, v), info, 0.001, DBL_MAX, 1, rnd))
			{
				const double dist2 = info.distance * info.distance;
				const double cosine = abs(v * normal); // don't use the normal from info because for three normals case it's not good for pdf

				return dist2 / (cosine * area);
			}

			return 0; 
		}

		// see Osada et All, Shape Distributions, section 4.2
		// http://www.cs.princeton.edu/~funk/tog02.pdf
		// another method is to simply generate the r1 and r2
		// and use r1 * edge1 + r2 * edge2 (+A) then reject those that are outside the triangle (half of them), retrying until success
		virtual Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override 
		{
			const double r1 = rnd.getZeroOne();
			const double r2 = rnd.getZeroOne();
			const double sr1 = sqrt(r1);
			const Vector3D<double> rndPoint((1. - sr1) * A + sr1 * (1. - r2) * B + sr1 * r2 * C);

			return rndPoint - origin; 
		}


		void SetUseInterpolation()
		{
			if (U1 >= 0 && V1 >= 0 && U2 >= 0 && V2 >= 0 && U3 >= 0 && V3 >= 0)
				useInterpolation = true;
			else
				useInterpolation = false;
		}

		Vector3D<double> A, B, C;

		double U1, V1, U2, V2, U3, V3;

	protected:
		bool threeNormals;

		double area;
		Vector3D<double> normal; // the computed normal, it's the real one

		// normals supplied by other means, for each vertex (for example by loading from an obj file or programmatically)
		// they are not necessarily the same as the normal above
		// they can be used to 'fake' a curved triangle, by computing a normal in the hit point using them, based on barycentric coordinates
		Vector3D<double> normal1;
		Vector3D<double> normal2;
		Vector3D<double> normal3;

		Vector3D<double> edge1;
		Vector3D<double> edge2;

		bool useInterpolation;
	};

}