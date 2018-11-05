#pragma once

#include "BVHNode.h"


namespace Objects {

	// contains a bunch of objects
	// they might have each one a material,
	// or share a common material

	class VisibleObjectComposite : public VisibleObject
	{
	protected:
		std::shared_ptr<BVH::BVHNode> root;
		BVH::AxisAlignedBoundingBox boundingBox;

	public:
		std::vector<std::shared_ptr<VisibleObject>> objects;

		VisibleObjectComposite() {}

		virtual bool IsComposite() const override { return true; }


		virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (root) return root->Hit(ray, info, minr, maxr, rcount, random);

			bool wasHit = false;
			PointInfo oinfo;
			oinfo.distance = DBL_MAX;
			info.distance = DBL_MAX;

			for (const auto& obj : objects)
				if (obj->Hit(ray, oinfo, minr, maxr, rcount, random))
				{
					wasHit = true;
					if (oinfo.distance < info.distance)
						info = oinfo;
				}


			return wasHit;
		}

		virtual bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
		{
			if (objects.empty()) return false;

			box = boundingBox;

			return true;
		}

		virtual void ConstructBoundingBox() override
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
		}

		virtual void ConstructBVH() override
		{
			ConstructBoundingBox();

			for (auto& obj : objects)
				obj->ConstructBVH();

			root = std::make_shared<BVH::BVHNode>(objects.begin(), objects.end());
		}

		virtual void Translate(const Vector3D<double>& t) override
		{
			for (auto& obj : objects)
				obj->Translate(t);

			boundingBox.Translate(t);
		}

		virtual void RotateAround(const Vector3D<double>& v, double angle) override
		{
			for (auto& obj : objects)
				obj->RotateAround(v, angle);

			ConstructBoundingBox();
		}


		virtual void Scale(double s) override
		{
			for (auto& obj : objects)
				obj->Scale(s);

			boundingBox.Scale(s);
		}


		virtual double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
		{							
			PointInfo info;

			if (Hit(Ray(o, v), info, 0.001, DBL_MAX, 1, rnd))
			{
				const double w = 1. / objects.size(); //equal weight for all of them, reasonably good if they are about the same size, if not, maybe something different should be used (based on area?)
				return w * info.object->pdfValue(o, v, rnd);
			}
			
			return 0;
		}

		virtual Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
		{
			// this assumes equal weight for all objects contained, it could be improved (see also above)			
			const unsigned int obj = static_cast<unsigned int>(rnd.getZeroOne() * static_cast<double>(objects.size()));

			return objects[obj]->getRandom(origin, rnd);
		}
	};


	class ConstantMedium : public VisibleObjectElementary
	{
	public:
		ConstantMedium() : density(0) { isotropic = std::make_shared<Materials::Isotropic>(); }
		ConstantMedium(const std::shared_ptr<VisibleObject>& b, const std::shared_ptr<Textures::Texture>& t, double d) : boundary(b), density(d)
		{
			isotropic = std::make_shared<Materials::Isotropic>(t); 

			ConstructBoundingBox();
			ConstructBVH();
		}

		virtual void ConstructBoundingBox() override
		{
			if (boundary) boundary->ConstructBoundingBox();
		}

		virtual void ConstructBVH() override
		{
			if (boundary) boundary->ConstructBVH();
		}


		virtual bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
		{
			if (!boundary) return false;

			return boundary->BoundingBox(box);
		}

		virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (!boundary) return false;

			PointInfo info1, info2;

			if (boundary->Hit(ray, info1, -DBL_MAX, DBL_MAX, rcount, random))
				if (boundary->Hit(ray, info2, info1.distance + 0.0001, DBL_MAX, rcount, random))
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
					const double hitDist = -1. / density * log(random.getZeroOne());

					if (hitDist < distInside)
					{
						info.distance = info1.distance + hitDist;
						info.normal = Vector3D<double>(0, 0, 1);
						info.position = ray(info.distance);
						info.material = isotropic.get();
						info.object = this;
						info.u = info.v = 0;
						return true;
					}
				}

			return false;
		}

		virtual void Translate(const Vector3D<double>& t) override
		{
			if (boundary)
				boundary->Translate(t);
		}

		virtual void RotateAround(const Vector3D<double>& v, double angle) override
		{
			if (boundary)
				boundary->RotateAround(v, angle);
		}

		virtual void Scale(double s) override
		{
			if (boundary)
				boundary->Scale(s);
		}

		double density;
	protected:
		std::shared_ptr<Materials::Isotropic> isotropic;
		std::shared_ptr<VisibleObject> boundary;
	};


}