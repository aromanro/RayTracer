#pragma once

#include "Vector3D.h"
#include "VisibleObject.h"
#include "Actions.h"
#include "OrthoNormalBasis.h"

#include <tuple>

namespace Objects {

	class Triangle : public VisibleObjectElementary
	{
	public:
		Triangle() : U1(-1), V1(-1), U2(-1), V2(-1), U3(-1), V3(-1), threeNormals(true), area(0), useInterpolation(true) {}
		Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const std::shared_ptr<Materials::Material>& m = nullptr, const std::shared_ptr<Textures::Texture>& t = nullptr);
		Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const Vector3D<double>& n1, const Vector3D<double>& n2, const Vector3D<double>& n3, const std::shared_ptr<Materials::Material>& m = nullptr, const std::shared_ptr<Textures::Texture>& t = nullptr);

		void InitTangents();

		void ConstructBoundingBox() override
		{
			Vector3D minv(std::min(std::min(A.X, B.X), C.X), std::min(std::min(A.Y, B.Y), C.Y), std::min(std::min(A.Z, B.Z), C.Z));
			Vector3D maxv(std::max(std::max(A.X, B.X), C.X), std::max(std::max(A.Y, B.Y), C.Y), std::max(std::max(A.Z, B.Z), C.Z));

			const Vector3D<double> dif = maxv - minv;

			if (dif.X < 1E-5) maxv.X += 1E-5;
			if (dif.Y < 1E-5) maxv.Y += 1E-5;
			if (dif.Z < 1E-5) maxv.Z += 1E-5;

			boundingBox = BVH::AxisAlignedBoundingBox(minv, maxv);
		}

		void Translate(const Vector3D<double>& t) override
		{
			A += t;
			B += t;
			C += t;

			boundingBox.Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
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

				tangent1 = tangent1.RotateAround(v, angle);
				tangent2 = tangent1.RotateAround(v, angle);
				tangent3 = tangent1.RotateAround(v, angle);

				bitangent1 = (tangent1 % normal1).Normalize(); // normalization is not really necessary
				bitangent2 = (tangent2 % normal2).Normalize(); // normalization is not really necessary
				bitangent3 = (tangent3 % normal3).Normalize(); // normalization is not really necessary
			}
			else
			{
				normal1 = normal2 = normal3 = normal;
				tangent1 = tangent2 = tangent3 = tangent1.RotateAround(v, angle);
				bitangent1 = bitangent2 = bitangent3 = (tangent1 % normal).Normalize(); // normalization is not really necessary
			}


			ConstructBoundingBox();
		}

		void Scale(double scale) override
		{
			A *= scale;
			B *= scale;
			C *= scale;
			edge1 *= scale;
			edge2 *= scale;
			
			area *= scale * scale;

			boundingBox.Scale(scale);
		}


		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override;


		inline static double Interpolate(const PointInfo& info, double val1, double val2, double val3)
		{
			const double w = 1. - info.u - info.v;

			return w * val1 + info.u * val2 + info.v * val3;
		}


		inline static std::pair<double, double> Interpolate(const PointInfo& info, const std::pair<double, double> v1, const std::pair<double, double> v2, const std::pair<double, double> v3)
		{
			const double w = 1. - info.u - info.v;

			return std::make_pair(w * v1.first + info.u * v2.first + info.v * v3.first, w * v1.second + info.u * v2.second + info.v * v3.second);
		}

		inline static Color Interpolate(const PointInfo& info, const Color& col1, const Color& col2, const Color& col3)
		{
			const double w = 1. - info.u - info.v;

			return w * col1 + info.u * col2 + info.v * col3;
		}

		double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override 
		{ 
			if (PointInfo info; Hit(Ray(o, v), info, 1E-5, DBL_MAX, 1, rnd))
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
		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override 
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

		inline Vector3D<double> getNormal(const PointInfo& info) const
		{
			const Vector3D<double> tNormal = getTriangleNormal(info);
			if (!GetNormals()) 
				return tNormal;

			const Color textureColor = GetNormals()->Value(info.u, info.v, info.position);
			const Vector3D bumpNormal(textureColor.r * 2. - 1., textureColor.g * 2. - 1., textureColor.b * 2. - 1.);

			Vector3D tangent = getTangent(info);
			Vector3D bitangent = getBitangent(info);

			// renormalize
			tangent -= (tangent * tNormal) * tNormal;
			bitangent -= (bitangent * tNormal) * tNormal;

			return OrthoNormalBasis(tNormal, tangent, bitangent).GlobalToLocal(bumpNormal).Normalize();
		}

		Vector3D<double> A;
		Vector3D<double> B;
		Vector3D<double> C;

		double U1 = -1;
		double V1 = -1;
		double U2 = -1;
		double V2 = -1;
		double U3 = -1;
		double V3 = -1;

	private:
		void Init();

		inline Vector3D<double> getTriangleNormal(const PointInfo& info) const
		{
			if (threeNormals)
			{
				const double w = 1. - info.u - info.v;

				return (w * normal1 + info.u * normal2 + info.v * normal3).Normalize();
			}

			return normal;
		}

		inline Vector3D<double> getTangent(const PointInfo& info) const
		{
			if (threeNormals)
			{
				const double w = 1. - info.u - info.v;

				return (w * tangent1 + info.u * tangent2 + info.v * tangent3).Normalize();
			}

			return tangent1;
		}

		inline Vector3D<double> getBitangent(const PointInfo& info) const
		{
			if (threeNormals)
			{
				const double w = 1. - info.u - info.v;

				return (w * bitangent1 + info.u * bitangent2 + info.v * bitangent3).Normalize();
			}

			return bitangent1;
		}

		bool threeNormals;

		double area;
		Vector3D<double> normal; // the computed normal, it's the real one

		// normals supplied by other means, for each vertex (for example by loading from an obj file or programmatically)
		// they are not necessarily the same as the normal above
		// they can be used to 'fake' a curved triangle, by computing a normal in the hit point using them, based on barycentric coordinates
		Vector3D<double> normal1;
		Vector3D<double> normal2;
		Vector3D<double> normal3;

		Vector3D<double> tangent1;
		Vector3D<double> tangent2;
		Vector3D<double> tangent3;

		Vector3D<double> bitangent1;
		Vector3D<double> bitangent2;
		Vector3D<double> bitangent3;

		Vector3D<double> edge1;
		Vector3D<double> edge2;

		bool useInterpolation = false;
	};

}