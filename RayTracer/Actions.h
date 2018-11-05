#pragma once

#include "VisibleObject.h"

namespace Transforms
{

	class FlipNormal : public Objects::VisibleObject
	{
	public:
		FlipNormal() {};
		FlipNormal(const std::shared_ptr<Objects::VisibleObject>& o) : obj(o) {}

		virtual bool IsComposite() const override { return obj->IsComposite(); }

		virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (obj->Hit(ray, info, minr, maxr, rcount, random))
			{
				info.normal *= -1;
				return true;
			}

			return false;
		}

		virtual bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
		{
			return obj->BoundingBox(box);

			return false;
		}


		virtual void Translate(const Vector3D<double>& t) override
		{
			obj->Translate(t);
		}

		virtual void RotateAround(const Vector3D<double>& v, double angle) override
		{
			obj->RotateAround(v, angle);
		}

		virtual void Scale(double s) override
		{
			obj->Scale(s);
		}

		virtual double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
		{
			return obj->pdfValue(o, v, rnd);
		}

		virtual Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
		{
			return obj->getRandom(origin, rnd);
		}

		std::shared_ptr<Objects::VisibleObject> obj;
	};

	class TranslateAction : public Objects::VisibleObject
	{
		public:
			TranslateAction() {};
			TranslateAction(const std::shared_ptr<Objects::VisibleObject>& o, const Vector3D<double>& off)
				: obj(o), offset(off)
			{
				offlen = off.Length();
			}

			virtual bool IsComposite() const override { return obj->IsComposite(); }


			virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
			{
				const Ray offRay(ray.getOrigin() - offset, ray.getDirection());

				if (obj->Hit(offRay, info, minr, maxr, rcount, random))
				{
					info.position += offset;

					return true;
				}

				return false;
			}

			virtual bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
			{
				if (obj->BoundingBox(box))
				{
					box = BVH::AxisAlignedBoundingBox(box.min() + offset, box.max() + offset);
					return true;
				}

				return false;
			}

			virtual void Translate(const Vector3D<double>& t) override
			{
				obj->Translate(t);					
			}

			virtual void RotateAround(const Vector3D<double>& v, double angle) override
			{
				obj->RotateAround(v, angle);
				offset.RotateAround(v, angle);
			}

			virtual void Scale(double s) override
			{
				obj->Scale(s);

				offset *= s;
				offlen *= s;
			}

			virtual double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
			{
				return obj->pdfValue(o - offset, v, rnd);
			}

			virtual Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
			{
				return obj->getRandom(origin - offset, rnd) + offset;
			}


	protected:
			std::shared_ptr<Objects::VisibleObject> obj;
			Vector3D<double> offset;
			double offlen;
	};

	class RotateYAction : public Objects::VisibleObject
	{
	public:
		RotateYAction() {};

		RotateYAction(const std::shared_ptr<Objects::VisibleObject>& o, double ang)
			: obj(o), angle(ang)
		{
			ConstructBoundingBox();
		}

		virtual void  ConstructBoundingBox() override
		{
			static const Vector3D<double> YAxis(0, 1, 0);
			hasBox = obj->BoundingBox(box);
			Vector3D<double> Min(DBL_MAX, DBL_MAX, DBL_MAX);
			Vector3D<double> Max(-DBL_MAX, -DBL_MAX, -DBL_MAX);
			for (int i = 0; i <= 1; ++i)
				for (int j = 0; j <= 1; ++j)
					for (int k = 0; k <= 1; ++k)
					{
						Vector3D<double> corner(i * box.min().X + (1 - i) * box.max().X, j * box.min().Y + (1 - j) * box.max().Y, k * box.min().Z + (1 - k) * box.max().Z);
						corner = corner.RotateAround(YAxis, angle);

						if (corner.X > Max.X) Max.X = corner.X;
						if (corner.X < Min.X) Min.X = corner.X;

						if (corner.Y > Max.Y) Max.Y = corner.Y;
						if (corner.Y < Min.Y) Min.Y = corner.Y;

						if (corner.Z > Max.Z) Max.Z = corner.Z;
						if (corner.Z < Min.Z) Min.Z = corner.Z;
					}

			box = BVH::AxisAlignedBoundingBox(Min, Max);
		}

		virtual bool IsComposite() const override { return obj->IsComposite(); }


		virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			static const Vector3D<double> YAxis(0, 1, 0);
			Ray rotatedRay(ray.getOrigin().RotateAround(YAxis, -angle), ray.getDirection().RotateAround(YAxis, -angle));

			if (obj->Hit(rotatedRay, info, minr, maxr, rcount, random))
			{
				info.position = info.position.RotateAround(YAxis, angle);
				info.normal = info.normal.RotateAround(YAxis, angle);
				
				return true;
			}

			return false;
		}

		virtual bool BoundingBox(BVH::AxisAlignedBoundingBox& rbox) override
		{
			rbox = box;

			return hasBox;
		}

		virtual void Translate(const Vector3D<double>& t) override
		{
			obj->Translate(t);
			ConstructBoundingBox();
		}

		virtual void RotateAround(const Vector3D<double>& v, double angle) override
		{
			obj->RotateAround(v, angle);
			ConstructBoundingBox();
		}

		virtual void Scale(double s) override
		{
			obj->Scale(s);

			box.Scale(s);
		}

		virtual double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
		{
			static const Vector3D<double> YAxis(0, 1, 0);

			return obj->pdfValue(o.RotateAround(YAxis, -angle), v.RotateAround(YAxis, -angle), rnd);
		}

		virtual Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
		{
			static const Vector3D<double> YAxis(0, 1, 0);

			return obj->getRandom(origin.RotateAround(YAxis, -angle), rnd).RotateAround(YAxis, angle);
		}

	protected:
		bool hasBox = false;
		double angle = 0;
		std::shared_ptr<Objects::VisibleObject> obj;
		BVH::AxisAlignedBoundingBox box;
	};
}

