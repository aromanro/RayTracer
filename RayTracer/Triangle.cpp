#include "Triangle.h"
#include "Material.h"

namespace Objects {

	Triangle::Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const std::shared_ptr<Materials::Material>& m, const std::shared_ptr<Textures::Texture>& t)
		: VisibleObjectElementary(m, t), A(a), B(b), C(c), U1(-1), V1(-1), U2(-1), V2(-1), U3(-1), V3(-1), threeNormals(false), useInterpolation(false)
	{
		Init();
	}


	Triangle::Triangle(const Vector3D<double>& a, const Vector3D<double>& b, const Vector3D<double>& c, const Vector3D<double>& n1, const Vector3D<double>& n2, const Vector3D<double>& n3, const std::shared_ptr<Materials::Material>& m, const std::shared_ptr<Textures::Texture>& t)
		: VisibleObjectElementary(m, t), A(a), B(b), C(c), U1(-1), V1(-1), U2(-1), V2(-1), U3(-1), V3(-1), threeNormals(true),
		normal1(n1), normal2(n2), normal3(n3), useInterpolation(false)
	{
		Init();
	}

	void Triangle::Init()
	{
		edge1 = B - A;
		edge2 = C - A;
		normal = edge1 % edge2;
		area = normal.Length() * 0.5;
		normal = normal.Normalize();

		if (!threeNormals)
			normal1 = normal2 = normal3 = normal;

		ConstructBoundingBox();
	}


	void Triangle::InitTangents()
	{
		Vector3D<double> tangent;

		if (U1 >= 0 && V1 >= 0 && U2 >= 0 && V2 >= 0 && U3 >= 0 && V3 >= 0)
		{
			const double dU1 = U2 - U1;
			const double dV1 = V2 - V1;
			const double dU2 = U3 - U1;
			const double dV2 = V3 - V1;

			const double f = 1. / (dU1 * dV2 - dU2 * dV1);

			tangent = Vector3D<double>(f * (dV2 * edge1.X - dV1 * edge2.X), f * (dV2 * edge1.Y - dV1 * edge2.Y), f * (dV2 * edge1.Z - dV1 * edge2.Z));
		}
		else
		{
			tangent = edge1 + edge2; // whatever
		}

		// this is tangent to the triangle, but not necessarily orthogonal to the vertex normal (because it might not be orthogonal to the triangle plane), so orthogonalize it:
		tangent = tangent.Normalize();
		// just subtract out the component along the normal, for each vertex, then normalize it again

		if (threeNormals)
		{
			Vector3D<double> vtangent = tangent - (tangent * normal1) * normal1;
			tangent1 = vtangent.Normalize();
			bitangent1 = (tangent1 % normal1).Normalize(); // normalization is not really necessary

			vtangent = tangent - (tangent * normal2) * normal2;
			tangent2 = vtangent.Normalize();
			bitangent2 = (tangent2 % normal2).Normalize(); // normalization is not really necessary

			vtangent = tangent - (tangent * normal3) * normal3;
			tangent3 = vtangent.Normalize();
			bitangent3 = (tangent3 % normal3).Normalize(); // normalization is not really necessary
		}
		else
		{
			tangent1 = tangent2 = tangent3 = tangent - (tangent * normal) * normal;
			bitangent1 = bitangent2 = bitangent3 = (tangent1 % normal).Normalize(); // normalization is not really necessary
		}
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
		info.material = GetMaterial().get();
		info.object = this;
		
		info.normal = getNormal(info);

		if (useInterpolation)
			std::tie(info.u, info.v) = Interpolate(info, std::make_pair(U1, V1), std::make_pair(U2, V2), std::make_pair(U3, V3));

		return true;
	}


}