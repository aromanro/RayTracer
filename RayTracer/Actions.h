#pragma once

#include "VisibleObject.h"

namespace Transforms
{

	class FlipNormal : public Objects::VisibleObject
	{
	public:
		FlipNormal() = default;
		explicit FlipNormal(const std::shared_ptr<Objects::VisibleObject>& o) : obj(o) {}

		bool IsComposite() const override { return obj->IsComposite(); }

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (obj->Hit(ray, info, minr, maxr, rcount, random))
			{
				info.normal *= -1;
				return true;
			}

			return false;
		}

		bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
		{
			return obj->BoundingBox(box);
		}

		void Translate(const Vector3D<double>& t) override
		{
			obj->Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			obj->RotateAround(v, angle);
		}

		void Scale(double s) override
		{
			obj->Scale(s);
		}

		double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
		{
			return obj->pdfValue(o, v, rnd);
		}

		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
		{
			return obj->getRandom(origin, rnd);
		}

		const std::shared_ptr<Objects::VisibleObject>& GetObject() const { return obj; }

	private:
		std::shared_ptr<Objects::VisibleObject> obj;
	};

	class TranslateAction : public Objects::VisibleObject
	{
		public:
			TranslateAction() : obj(nullptr), offlen(0) {};
			TranslateAction(const std::shared_ptr<Objects::VisibleObject>& o, const Vector3D<double>& off)
				: obj(o), offset(off), offlen(off.Length())
			{
			}

			bool IsComposite() const override { return obj->IsComposite(); }


			bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
			{
				if (const Ray offRay(ray.getOrigin() - offset, ray.getDirection()); obj->Hit(offRay, info, minr, maxr, rcount, random))
				{
					info.position += offset;

					return true;
				}

				return false;
			}

			bool BoundingBox(BVH::AxisAlignedBoundingBox& box) override
			{
				if (obj->BoundingBox(box))
				{
					box = BVH::AxisAlignedBoundingBox(box.min() + offset, box.max() + offset);
					return true;
				}

				return false;
			}

			void Translate(const Vector3D<double>& t) override
			{
				obj->Translate(t);					
			}

			void RotateAround(const Vector3D<double>& v, double angle) override
			{
				obj->RotateAround(v, angle);
				offset.RotateAround(v, angle);
			}

			void Scale(double s) override
			{
				obj->Scale(s);

				offset *= s;
				offlen *= s;
			}

			double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
			{
				return obj->pdfValue(o - offset, v, rnd);
			}

			Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
			{
				return obj->getRandom(origin - offset, rnd) + offset;
			}


	private:
			std::shared_ptr<Objects::VisibleObject> obj;
			Vector3D<double> offset;
			double offlen;
	};

	class RotateYAction : public Objects::VisibleObject
	{
	public:
		RotateYAction() = default;

		RotateYAction(const std::shared_ptr<Objects::VisibleObject>& o, double ang)
			: angle(ang), obj(o)
		{
			InitBoundingBox();
		}

		void ConstructBoundingBox() override
		{
			InitBoundingBox();
		}

		bool IsComposite() const override { return obj->IsComposite(); }


		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			static const Vector3D<double> YAxis(0, 1, 0);
			
			if (Ray rotatedRay(ray.getOrigin().RotateAround(YAxis, -angle), ray.getDirection().RotateAround(YAxis, -angle)); obj->Hit(rotatedRay, info, minr, maxr, rcount, random))
			{
				info.position = info.position.RotateAround(YAxis, angle);
				info.normal = info.normal.RotateAround(YAxis, angle);
				
				return true;
			}

			return false;
		}

		bool BoundingBox(BVH::AxisAlignedBoundingBox& rbox) override
		{
			rbox = box;

			return hasBox;
		}

		void Translate(const Vector3D<double>& t) override
		{
			obj->Translate(t);
			ConstructBoundingBox();
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			obj->RotateAround(v, angle);
			ConstructBoundingBox();
		}

		void Scale(double s) override
		{
			obj->Scale(s);

			box.Scale(s);
		}

		double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
		{
			static const Vector3D<double> YAxis(0, 1, 0);

			return obj->pdfValue(o.RotateAround(YAxis, -angle), v.RotateAround(YAxis, -angle), rnd);
		}

		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override
		{
			static const Vector3D<double> YAxis(0, 1, 0);

			return obj->getRandom(origin.RotateAround(YAxis, -angle), rnd).RotateAround(YAxis, angle);
		}

	private:
		void InitBoundingBox()
		{
			static const Vector3D<double> YAxis(0, 1, 0);
			hasBox = obj->BoundingBox(box);
			Vector3D Min(DBL_MAX, DBL_MAX, DBL_MAX);
			Vector3D Max(-DBL_MAX, -DBL_MAX, -DBL_MAX);

			for (int i = 0; i <= 1; ++i)
				for (int j = 0; j <= 1; ++j)
					for (int k = 0; k <= 1; ++k)
					{
						Vector3D corner(i * box.min().X + (1LL - i) * box.max().X, j * box.min().Y + (1LL - j) * box.max().Y, k * box.min().Z + (1LL - k) * box.max().Z);
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

		bool hasBox = false;
		double angle = 0;
		std::shared_ptr<Objects::VisibleObject> obj;
		BVH::AxisAlignedBoundingBox box;
	};
}

