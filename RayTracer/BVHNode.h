#pragma once

#include "VisibleObject.h"
#include "AxisAlignedBoundingBox.h"
#include "Random.h"

#include <algorithm>

namespace BVH
{

	class BVHNode : public Objects::VisibleObject
	{
	public:
		BVHNode() = default;

		BVHNode(std::vector<std::shared_ptr<Objects::VisibleObject>>::iterator start, std::vector<std::shared_ptr<Objects::VisibleObject>>::iterator end, int dir = 0)
		{
			const int size = static_cast<int>(end - start);
			if (0 == size) return;

			if (0 == dir)
			{
				std::sort(start, end, [](const auto& o1, const auto& o2)
				{
					AxisAlignedBoundingBox b1;
					AxisAlignedBoundingBox b2;

					o1->BoundingBox(b1);
					o2->BoundingBox(b2);

					return b1.min().X < b2.min().X;
				});
			}
			else if (1 == dir)
			{
				std::sort(start, end, [](const auto& o1, const auto& o2)
				{
					AxisAlignedBoundingBox b1;
					AxisAlignedBoundingBox b2;

					o1->BoundingBox(b1);
					o2->BoundingBox(b2);

					return b1.min().Y < b2.min().Y;
				});
			}
			else
			{
				std::sort(start, end, [](const auto& o1, const auto& o2)
				{
					AxisAlignedBoundingBox b1;
					AxisAlignedBoundingBox b2;

					o1->BoundingBox(b1);
					o2->BoundingBox(b2);

					return b1.min().Z < b2.min().Z;
				});
			}

			if (1 == size)
			{
				child1 = *start;
				child1->BoundingBox(boundingBox);
			}
			else if (2 == size)
			{
				child1 = *start;
				child2 = *(start + 1);

				AxisAlignedBoundingBox b1;
				AxisAlignedBoundingBox b2;
				child1->BoundingBox(b1);
				child2->BoundingBox(b2);
				boundingBox = AxisAlignedBoundingBox::EnclosingBox(b1, b2);
			}
			else
			{
				if (++dir > 2) dir = 0;

				const int mIndex = size / 2;
				child1 = std::dynamic_pointer_cast<Objects::VisibleObject>(std::make_shared<BVHNode>(start, start + mIndex, dir));
				child2 = std::dynamic_pointer_cast<Objects::VisibleObject>(std::make_shared<BVHNode>(start + mIndex, end, dir));

				AxisAlignedBoundingBox b1;
				AxisAlignedBoundingBox b2;
				child1->BoundingBox(b1);
				child2->BoundingBox(b2);
				boundingBox = AxisAlignedBoundingBox::EnclosingBox(b1, b2);
			}
		}

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (boundingBox.Intersects(ray, minr, maxr))
			{
				if (child1->Hit(ray, info, minr, maxr, rcount, random))
				{
					if (child2) child2->Hit(ray, info, minr, info.distance, rcount, random);

					return true;
				}
				else if (child2)
					return child2->Hit(ray, info, minr, maxr, rcount, random);				
			}

			return false;
		}

		bool BoundingBox(AxisAlignedBoundingBox& box) const override
		{
			box = boundingBox;

			return true;
		}


		void Translate(const Vector3D<double>& t) override
		{
			boundingBox.Translate(t);

			child1->Translate(t);
			if (child2) child2->Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			child1->RotateAround(v, angle);
			child1->BoundingBox(boundingBox);

			if (child2)
			{
				AxisAlignedBoundingBox b;
				child2->RotateAround(v, angle);
				child2->BoundingBox(b);

				boundingBox = AxisAlignedBoundingBox::EnclosingBox(boundingBox, b);
			}
		}

		void Scale(double s) override
		{
			boundingBox.Scale(s);
			child1->Scale(s);
			if (child2) child2->Scale(s);
		}

	private:
		AxisAlignedBoundingBox boundingBox;

		// only leaves are something else than BVHNode
		std::shared_ptr<VisibleObject> child1;
		std::shared_ptr<VisibleObject> child2;
	};


}
