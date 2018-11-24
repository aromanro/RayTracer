#include "Triangle.h"


namespace Objects {

	Triangle::Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const std::shared_ptr<Materials::Material>& m)
		: VisibleObjectElementary(m), A(a), B(b), C(c), U1(-1), V1(-1), U2(-1), V2(-1), U3(-1), V3(-1), threeNormals(false), useInterpolation(false)
	{
		edge1 = B - A;
		edge2 = C - A;
		normal = edge1 % edge2;
		area = normal.Length() * 0.5;
		normal = normal.Normalize();

		ConstructBoundingBox();
	}


	Triangle::Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const Vector3D<double>& n1, const Vector3D<double>& n2, const Vector3D<double>& n3, const std::shared_ptr<Materials::Material>& m)
		: VisibleObjectElementary(m), A(a), B(b), C(c), U1(-1), V1(-1), U2(-1), V2(-1), U3(-1), V3(-1), threeNormals(true),
		normal1(n1), normal2(n2), normal3(n3), useInterpolation(false)
	{
		edge1 = B - A;
		edge2 = C - A;
		const Vector3D<double> Normal = edge1 % edge2;
		area = Normal.Length() * 0.5;
		normal = Normal.Normalize();

		ConstructBoundingBox();
	}

	// Fast, minimum storage ray-triangle intersection.
	// Tomas Möller and Ben Trumbore. 
	// Journal of Graphics Tools, 2(1):21--28, 1997. 
	// http://www.graphics.cornell.edu/pubs/1997/MT97.pdf

	bool Triangle::Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const
	{
		const Vector3D<double> pvec = ray.getDirection() % edge2;
		const double det = pvec * edge1;

		// non-culling
		if (abs(det) < 1E-15) return false;

		const double invdet = 1. / det;

		const Vector3D<double> tvec = ray.getOrigin() - A;

		const double u = pvec * tvec * invdet;
		if (u < 0 || u > 1) return false; 

		const Vector3D<double> qvec = tvec % edge1;
		const double v = qvec * ray.getDirection() * invdet;

		if (v < 0 || u + v > 1.) return false;

		const double t = qvec * edge2 * invdet; 

		if (t < minr || t > maxr) return false;

		info.distance = t;
		info.position = ray.getOrigin() + t * ray.getDirection();
		info.u = u;
		info.v = v;
		info.material = material.get();
		info.object = this;
		
		if (threeNormals)
		{
			const double w = 1. - u - v;
			info.normal = (w * normal1 + u * normal2 + v * normal3).Normalize();
		}
		else info.normal = normal;

		if (useInterpolation)
			std::tie(info.u, info.v) = Interpolate(info, std::make_pair(U1, V1), std::make_pair(U2, V2), std::make_pair(U3, V3));
		
		return true;
	}


}