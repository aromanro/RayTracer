#pragma once

#include "BVHNode.h"


namespace Objects {

	// contains a bunch of objects
	// they might have each one a material,
	// or share a common material

	class VisibleObjectComposite : public VisibleObject
	{
	public:
		std::vector<std::shared_ptr<VisibleObject>> objects;

		VisibleObjectComposite() : w(0.) {}

		bool IsComposite() const override { return true; }

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (root) return root->Hit(ray, info, minr, maxr, rcount, random);

			bool wasHit = false;

			PointInfo oinfo;
			PointInfo hitInfo;
			hitInfo.distance = oinfo.distance = maxr;

			for (int i = 0; i < objects.size(); ++i)
				if (objects[i]->Hit(ray, oinfo, minr, hitInfo.distance, rcount, random))
				{
					wasHit = true;
					if (oinfo.distance < hitInfo.distance)
						hitInfo = oinfo;
				}

			if (wasHit) info = hitInfo;

			return wasHit;
		}

		bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
		{
			if (objects.empty()) return false;

			box = boundingBox;

			return true;
		}

		void ConstructBoundingBox() override
		{
			if (objects.empty()) return;

			BVH::AxisAlignedBoundingBox res;

			objects[0]->ConstructBoundingBox();
			objects[0]->BoundingBox(res);
			boundingBox = res;

			for (int i = 1; i < objects.size(); ++i)
			{
				objects[i]->ConstructBoundingBox();
				if (objects[i]->BoundingBox(res))
					boundingBox = BVH::AxisAlignedBoundingBox::EnclosingBox(boundingBox, res);
			}

			//equal weight for all of them, reasonably good if they are about the same size, if not, maybe something different should be used (based on area?)
			w = 1. / static_cast<double>(objects.size());
		}

		void ConstructBVH() override
		{
			ConstructBoundingBox();

			for (const auto& obj : objects)
				obj->ConstructBVH();

			root = std::make_shared<BVH::BVHNode>(objects.begin(), objects.end());
		}

		void Translate(const Vector3D<double>& t) override
		{
			for (const auto& obj : objects)
				obj->Translate(t);

			boundingBox.Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			for (const auto& obj : objects)
				obj->RotateAround(v, angle);

			ConstructBoundingBox();
		}


		void Scale(double s) override
		{
			for (const auto& obj : objects)
				obj->Scale(s);

			boundingBox.Scale(s);
		}


		double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
		{							
			if (PointInfo info; Hit(Ray(o, v), info, 1E-5, DBL_MAX, 1, rnd) && info.object)
				return w * info.object->pdfValue(o, v, rnd);
			
			return 0;
		}

		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
		{
			// this assumes equal weight for all objects contained, it could be improved (see also above)			
			const unsigned int obj = static_cast<unsigned int>(rnd.getZeroOne() * static_cast<double>(objects.size()));

			return objects[obj]->getRandom(origin, rnd);
		}

	protected:
		std::shared_ptr<BVH::BVHNode> root;
		BVH::AxisAlignedBoundingBox boundingBox;
		double w;
	};


	class ConstantMedium : public VisibleObjectElementary
	{
	public:
		ConstantMedium() : density(0), invDensity(1E15) { isotropic = std::make_shared<Materials::Isotropic>(); }
		
		ConstantMedium(const std::shared_ptr<VisibleObject>& b, const std::shared_ptr<Textures::Texture>& t, double d) 
			: density(d), invDensity(1. / density), boundary(b)
		{
			isotropic = std::make_shared<Materials::Isotropic>(t); 

			if (boundary)
			{
				boundary->ConstructBoundingBox();
				boundary->ConstructBVH();
			}
		}

		void ConstructBoundingBox() override
		{
			if (boundary) boundary->ConstructBoundingBox();
		}

		void ConstructBVH() override
		{
			if (boundary) boundary->ConstructBVH();
		}


		bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
		{
			if (!boundary) return false;

			return boundary->BoundingBox(box);
		}

		inline Vector3D<double> getNormal(const PointInfo& /*info*/) const
		{
			static const Vector3D<double> normal(0, 0, 1);

			return normal;
		}

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (!boundary) return false;

			if (PointInfo info1; boundary->Hit(ray, info1, -DBL_MAX, DBL_MAX, rcount, random))
				if (PointInfo info2; boundary->Hit(ray, info2, info1.distance + 0.000001, DBL_MAX, rcount, random))
				{
					if (info1.distance < minr)
						info1.distance = minr;
					if (info2.distance > maxr)
						info2.distance = maxr;

					if (info1.distance >= info2.distance)
						return false;

					if (info1.distance < 0)
						info1.distance = 0;

					const double distInside = info2.distance - info1.distance;
					const double hitDist = -invDensity * log(random.getZeroOne());

					if (hitDist < distInside)
					{
						info.distance = info1.distance + hitDist;
						info.normal = getNormal(info);
						info.position = ray(info.distance);
						info.material = isotropic.get();
						info.object = this;
						info.u = info.v = 0;
						return true;
					}
				}

			return false;
		}

		void Translate(const Vector3D<double>& t) override
		{
			if (boundary)
				boundary->Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			if (boundary)
				boundary->RotateAround(v, angle);
		}

		void Scale(double s) override
		{
			if (boundary)
				boundary->Scale(s);
		}

		double getDensity() const { return density; }
		
		void setDensity(double d)
		{
			density = d;
			invDensity = 1. / d;
		}

	protected:
		double density;
		double invDensity;
		std::shared_ptr<Materials::Isotropic> isotropic;
		std::shared_ptr<VisibleObject> boundary;
	};


}