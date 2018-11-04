#pragma once

#include "Ray.h"
#include "Color.h"
#include "AxisAlignedBoundingBox.h"
#include "Random.h"

#include <vector>
#include <memory>
#include <algorithm>



namespace Materials {
	class Material;
}

namespace Objects {
	class VisibleObject;
}

class PointInfo
{
public:
	PointInfo() : distance(0), u(0), v(0), material(NULL) {}

	double distance;
	Vector3D<double> normal;
	Vector3D<double> position;
	double u;
	double v;

	Materials::Material* material;
	const Objects::VisibleObject* object;
};




namespace Objects {

	// derive from it triangles, quads, sphere, cube, whatever

	class VisibleObject
	{
	protected:
		VisibleObject() {};
	public:
		virtual ~VisibleObject() {};

		virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const = 0;
		virtual bool BoundingBox(BVH::AxisAlignedBoundingBox& box) = 0;
		virtual void ConstructBoundingBox() {};
		virtual void ConstructBVH() { ConstructBoundingBox(); };

		virtual void Translate(const Vector3D<double>& t) = 0;
		virtual void RotateAround(const Vector3D<double>& v, double angle) = 0;
		virtual void Scale(double s) = 0;

		virtual double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const { return 0; }
		virtual Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const { return Vector3D<double>(0, 0, 1); }
	};


	class VisibleObjectMaterial : public VisibleObject
	{
	public:
		void SetMaterial(const std::shared_ptr<Materials::Material>& m) { material = m; }
	protected:
		VisibleObjectMaterial() {};
		VisibleObjectMaterial(const std::shared_ptr<Materials::Material>& m) : material(m) {};

		std::shared_ptr<Materials::Material> material;
	};


	class VisibleObjectElementary : public VisibleObjectMaterial
	{
	protected:
		VisibleObjectElementary() {};
		VisibleObjectElementary(const std::shared_ptr<Materials::Material>& m) : VisibleObjectMaterial(m) {};

		BVH::AxisAlignedBoundingBox boundingBox;
	public:
		virtual bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
		{
			box = boundingBox;

			return true;
		}
	};

}

