#pragma once


#include "VisibleObject.h"
#include "Actions.h"
#include "ImageTexture.h"

namespace Objects
{

	// those are very limited, only special orientations, no rotation possible
	// just to implement a Cornell box with them

	class AlignedRectangle : public VisibleObjectElementary
	{
	public:
		AlignedRectangle() = default;
		explicit AlignedRectangle(const std::shared_ptr<Materials::Material>& m) : VisibleObjectElementary(m) {}

		double pdfValue(const Vector3D<double>& o, const Vector3D<double>& v, Random& rnd) const override
		{
			if (PointInfo info; Hit(Ray(o, v), info, 0.000001, DBL_MAX, 1, rnd))
			{
				const double c = v * info.normal;
				const double dist2 = info.distance * info.distance;
				const double cosine = abs(c);

				return dist2 / (cosine * area);
			}

			return 0;
		}

	protected:
		double dim1 = 0;
		double dim2 = 0;
		double invdim1 = 0;
		double invdim2 = 0;
		double area = 0;
	};

	class RectangleXY : public AlignedRectangle
	{
	public:
		RectangleXY() : m_z(0), m_x0(0), m_x1(0), m_y0(0), m_y1(0) {};
		RectangleXY(double x0, double x1, double y0, double y1, double z, const std::shared_ptr<Materials::Material>& m)
			: AlignedRectangle(m), m_z(z), m_x0(x0), m_x1(x1), m_y0(y0), m_y1(y1)
		{
			dim1 = m_x1 - m_x0;
			dim2 = m_y1 - m_y0;
			area = abs(dim1 * dim2);
			invdim1 = 1. / dim1;
			invdim2 = 1. / dim2;
			boundingBox = BVH::AxisAlignedBoundingBox(Vector3D<double>(m_x0, m_y0, m_z - 0.000001), Vector3D<double>(m_x1, m_y1, m_z + 0.000001));
		};

		inline Vector3D<double> getNormal(const PointInfo& /*info*/) const
		{
			static const Vector3D normal(0., 0., 1.);

			return normal;
		}

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			const double t = (m_z - ray.getOrigin().Z) * ray.getInvDir().Z;
			if (t < minr || t > maxr) return false;

			const double x = ray.getOrigin().X + t * ray.getDirection().X;
			const double y = ray.getOrigin().Y + t * ray.getDirection().Y;

			if (x < m_x0 || x > m_x1 || y < m_y0 || y > m_y1) return false;

			info.distance = t;
			info.position = ray(t);
			info.material = GetMaterial().get();
			info.object = this;
			info.normal = getNormal(info);

			info.u = (x - m_x0) * invdim1;
			info.v = (y - m_y0) * invdim2;

			return true;
		}

		void ConstructBoundingBox() override
		{
			boundingBox = BVH::AxisAlignedBoundingBox(Vector3D<double>(m_x0, m_y0, m_z - 0.000001), Vector3D<double>(m_x1, m_y1, m_z + 0.000001));
		}

		void Translate(const Vector3D<double>& t) override
		{
			m_z += t.Z;
			m_x0 += t.X;
			m_x1 += t.X;
			m_y0 += t.Y;
			m_y1 += t.Y;

			boundingBox.Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			// for now, don't rotate them, need a different implementation for rotation
		}

		void Scale(double s) override
		{
			m_z *= s;
			m_x0 *= s;
			m_x1 *= s;
			m_y0 *= s;
			m_y1 *= s;

			boundingBox.Scale(s);
		}
		
		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override 
		{ 
			return Vector3D<double>(m_x0 + rnd.getZeroOne() * dim1, m_y0 + rnd.getZeroOne() * dim2, m_z + 0.000001) - origin;
		}

		double m_z;
		double m_x0;
		double m_x1;
		double m_y0;
		double m_y1;
	};



	class RectangleXZ : public AlignedRectangle
	{
	public:
		RectangleXZ() : m_y(0), m_x0(0), m_x1(0), m_z0(0), m_z1(0) {};
		RectangleXZ(double x0, double x1, double z0, double z1, double y, const std::shared_ptr<Materials::Material>& m)
			: AlignedRectangle(m), m_y(y), m_x0(x0), m_x1(x1), m_z0(z0), m_z1(z1)
		{
			dim1 = m_x1 - m_x0;
			dim2 = m_z1 - m_z0;
			area = abs(dim1 * dim2);
			invdim1 = 1. / dim1;
			invdim2 = 1. / dim2;
			boundingBox = BVH::AxisAlignedBoundingBox(Vector3D<double>(m_x0, m_y - 0.000001, m_z0), Vector3D<double>(m_x1, m_y + 0.000001, m_z1));
		};

		inline Vector3D<double> getNormal(const PointInfo& /*info*/) const
		{
			static const Vector3D normal(0., 1., 0.);

			return normal;
		}

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			const double t = (m_y - ray.getOrigin().Y) * ray.getInvDir().Y;
			if (t < minr || t > maxr) return false;

			const double x = ray.getOrigin().X + t * ray.getDirection().X;
			const double z = ray.getOrigin().Z + t * ray.getDirection().Z;

			if (x < m_x0 || x > m_x1 || z < m_z0 || z > m_z1) return false;

			info.distance = t;
			info.position = ray(t);
			info.material = GetMaterial().get();
			info.object = this;
			info.normal = getNormal(info);

			info.u = (x - m_x0) * invdim1;
			info.v = (z - m_z0) * invdim2;

			return true;
		}

		void ConstructBoundingBox() override
		{
			boundingBox = BVH::AxisAlignedBoundingBox(Vector3D<double>(m_x0, m_y - 0.000001, m_z0), Vector3D<double>(m_x1, m_y + 0.000001, m_z1));
		}

		void Translate(const Vector3D<double>& t) override
		{
			m_y += t.Y;
			m_x0 += t.X;
			m_x1 += t.X;
			m_z0 += t.Z;
			m_z1 += t.Z;

			boundingBox.Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			// for now, don't rotate them, need a different implementation for rotation
		}


		void Scale(double s) override
		{
			m_y *= s;
			m_x0 *= s;
			m_x1 *= s;
			m_z0 *= s;
			m_z1 *= s;

			boundingBox.Scale(s);
		}
		
		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override 
		{ 
			return Vector3D<double>(m_x0 + rnd.getZeroOne() * dim1, m_y + 0.000001, m_z0 + rnd.getZeroOne() * dim2) - origin;
		}

		double m_y;
		double m_x0;
		double m_x1;
		double m_z0;
		double m_z1;
	};


	class RectangleYZ : public AlignedRectangle
	{
	public:
		RectangleYZ() : m_x(0), m_y0(0), m_y1(0), m_z0(0), m_z1(0) {};
		RectangleYZ(double y0, double y1, double z0, double z1, double x, const std::shared_ptr<Materials::Material>& m)
			: AlignedRectangle(m), m_x(x), m_y0(y0), m_y1(y1), m_z0(z0), m_z1(z1)
		{
			dim1 = m_y1 - m_y0;
			dim2 = m_z1 - m_z0;
			area = abs(dim1 * dim2);
			invdim1 = 1. / dim1;
			invdim2 = 1. / dim2;
			boundingBox = BVH::AxisAlignedBoundingBox(Vector3D<double>(m_x - 0.000001, m_y0, m_z0), Vector3D<double>(m_x + 0.000001, m_y1, m_z1));
		};

		inline Vector3D<double> getNormal(const PointInfo& /*info*/) const
		{
			static const Vector3D normal(1., 0., 0.);

			return normal;
		}

		bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			const double t = (m_x - ray.getOrigin().X) * ray.getInvDir().X;
			if (t < minr || t > maxr) return false;

			const double y = ray.getOrigin().Y + t * ray.getDirection().Y;
			const double z = ray.getOrigin().Z + t * ray.getDirection().Z;

			if (y < m_y0 || y > m_y1 || z < m_z0 || z > m_z1) return false;


			info.distance = t;
			info.position = ray(t);
			info.material = GetMaterial().get();
			info.object = this;
			info.normal = getNormal(info);

			info.u = (y - m_y0) * invdim1;
			info.v = (z - m_z0) * invdim2;

			return true;
		}

		void ConstructBoundingBox() override
		{
			boundingBox = BVH::AxisAlignedBoundingBox(Vector3D<double>(m_x - 0.000001, m_y0, m_z0), Vector3D<double>(m_x + 0.000001, m_y1, m_z1));
		}

		void Translate(const Vector3D<double>& t) override
		{
			m_x += t.X;
			m_y0 += t.Y;
			m_y1 += t.Y;
			m_z0 += t.Z;
			m_z1 += t.Z;

			boundingBox.Translate(t);
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			// for now, don't rotate them, need a different implementation for rotation
		}

		void Scale(double s) override
		{
			m_x *= s;
			m_y0 *= s;
			m_y1 *= s;
			m_z0 *= s;
			m_z1 *= s;

			boundingBox.Scale(s);
		}
		
		Vector3D<double> getRandom(const Vector3D<double>& origin, Random& rnd) const override 
		{ 
			return Vector3D<double>(m_x + 0.000001, m_y0 + rnd.getZeroOne() * dim1, m_z0 + rnd.getZeroOne() * dim2) - origin;
		}

		double m_x;
		double m_y0;
		double m_y1;
		double m_z0;
		double m_z1;
	};


	class Box : public VisibleObjectComposite
	{
	public:
		Box() = default;
		Box(const Vector3D<double>& p1, const Vector3D<double>& p2, const std::shared_ptr<Materials::Material>& m)
			: m_p1(p1), m_p2(p2)
		{
			AddFaces(m);

			boundingBox = BVH::AxisAlignedBoundingBox(m_p1, m_p2);
		};

		Box(const Vector3D<double>& p1, const Vector3D<double>& p2, const std::shared_ptr<Materials::Material>& front_mat, const std::shared_ptr<Materials::Material>& back_mat, const std::shared_ptr<Materials::Material>& ceil_mat, const std::shared_ptr<Materials::Material>& floor_mat, const std::shared_ptr<Materials::Material>& left_mat, const std::shared_ptr<Materials::Material>& right_mat)
			: m_p1(p1), m_p2(p2)
		{
			AddFaces(front_mat, back_mat, ceil_mat, floor_mat, left_mat, right_mat);

			boundingBox = BVH::AxisAlignedBoundingBox(m_p1, m_p2);
		};

		void ConstructBoundingBox() override
		{
			boundingBox = BVH::AxisAlignedBoundingBox(m_p1, m_p2);
		}

		void Translate(const Vector3D<double>& t) override
		{
			m_p1 += t;
			m_p2 += t;

			VisibleObjectComposite::Translate(t);

			ConstructBoundingBox();
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			// for now, don't rotate them, need a different implementation for rotation
		}

		void Scale(double s) override
		{
			m_p1 *= s;
			m_p2 *= s;

			boundingBox.Scale(s);
		}


		Vector3D<double> getCenter() const { return (m_p1 + m_p2) * 0.5; }
		double getRadius() const { return (m_p1 - getCenter()).Length(); }

	private:
		void AddFaces(const std::shared_ptr<Materials::Material>& material)
		{
			objects.emplace_back(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p2.Z, material)); // front
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p1.Z, material))); // back
			
			objects.emplace_back(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p2.Y, material)); // ceiling
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p1.Y, material))); // floor
			
			objects.emplace_back(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p2.X, material)); // left
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p1.X, material))); //right
		}

		void AddFaces(const std::shared_ptr<Materials::Material>& front_mat, const std::shared_ptr<Materials::Material>& back_mat, const std::shared_ptr<Materials::Material>& ceil_mat, const std::shared_ptr<Materials::Material>& floor_mat, const std::shared_ptr<Materials::Material>& left_mat, const std::shared_ptr<Materials::Material>& right_mat)
		{
			objects.emplace_back(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p2.Z, front_mat)); // front
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p1.Z, back_mat))); // back
			
			objects.emplace_back(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p2.Y, ceil_mat)); // ceiling
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p1.Y, floor_mat))); // floor
			
			objects.emplace_back(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p2.X, left_mat)); // left
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p1.X, right_mat))); //right
		}

		Vector3D<double> m_p1;
		Vector3D<double> m_p2;
	};




	class SkyBox : public VisibleObjectComposite
	{
	public:
		SkyBox() = default;

		SkyBox(const Vector3D<double>& p1, const Vector3D<double>& p2)
			: m_p1(p1), m_p2(p2)
		{
			AddFaces();
			boundingBox = BVH::AxisAlignedBoundingBox(m_p1, m_p2);
		}

		SkyBox(const Vector3D<double>& p1, const Vector3D<double>& p2, const std::shared_ptr<Materials::Material>& front_mat, const std::shared_ptr<Materials::Material>& back_mat, const std::shared_ptr<Materials::Material>& ceil_mat, const std::shared_ptr<Materials::Material>& floor_mat, const std::shared_ptr<Materials::Material>& left_mat, const std::shared_ptr<Materials::Material>& right_mat)			
		: m_p1(p1), m_p2(p2)
		{
			AddFaces(front_mat, back_mat, ceil_mat, floor_mat, left_mat, right_mat);
			boundingBox = BVH::AxisAlignedBoundingBox(m_p1, m_p2);
		};

		void ConstructBoundingBox() override
		{
			boundingBox = BVH::AxisAlignedBoundingBox(m_p1, m_p2);
		}

		void Translate(const Vector3D<double>& t) override
		{
			m_p1 += t;
			m_p2 += t;

			VisibleObjectComposite::Translate(t);

			ConstructBoundingBox();
		}

		void RotateAround(const Vector3D<double>& v, double angle) override
		{
			// for now, don't rotate them, need a different implementation for rotation
		}

		void Scale(double s) override
		{
			m_p1 *= s;
			m_p2 *= s;

			boundingBox.Scale(s);
		}


		Vector3D<double> getCenter() const { return (m_p1 + m_p2) * 0.5; }
		double getRadius() const { return (m_p1 - getCenter()).Length(); }



		bool Load(const std::string& front, const std::string& back, const std::string& ceil, const std::string& floor, const std::string& left, const std::string& right)
		{
			if (objects.size() < 6) return false;

			try
			{
				// front
				auto tf = std::make_shared<Textures::ImageTexture>(front);

				std::dynamic_pointer_cast<Objects::RectangleXY>(std::dynamic_pointer_cast<Transforms::FlipNormal>(objects[0])->GetObject())->SetMaterial(std::make_shared<Materials::Lambertian>(tf));


				// back
				auto tb = std::make_shared<Textures::ImageTexture>(back);
				tb->flipHorizontally = true;

				std::dynamic_pointer_cast<Objects::RectangleXY>(objects[1])->SetMaterial(std::make_shared<Materials::Lambertian>(tb));


				// ceiling
				auto tc = std::make_shared<Textures::ImageTexture>(ceil);
				tc->flipVertically = true;

				std::dynamic_pointer_cast<Objects::RectangleXZ>(std::dynamic_pointer_cast<Transforms::FlipNormal>(objects[2])->GetObject())->SetMaterial(std::make_shared<Materials::Lambertian>(tc));


				// floor - this might need some transformations
				auto texfloor = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ImageTexture>(floor));

				std::dynamic_pointer_cast<Objects::RectangleXZ>(objects[3])->SetMaterial(std::make_shared<Materials::Lambertian>(texfloor));


				// right
				auto tr = std::make_shared<Textures::ImageTexture>(right);
				tr->flipVertically = true;
				tr->flipAxes = true;


				std::dynamic_pointer_cast<Objects::RectangleYZ>(std::dynamic_pointer_cast<Transforms::FlipNormal>(objects[4])->GetObject())->SetMaterial(std::make_shared<Materials::Lambertian>(tr));


				// left
				auto tl = std::make_shared<Textures::ImageTexture>(left);
				tl->flipAxes = true;

				std::dynamic_pointer_cast<Objects::RectangleYZ>(objects[5])->SetMaterial(std::make_shared<Materials::Lambertian>(tl));
			}
			catch (...)
			{
				return false;
			}

			return true;
		}


	private:
		void AddFaces(const std::shared_ptr<Materials::Material>& front_mat, const std::shared_ptr<Materials::Material>& back_mat, const std::shared_ptr<Materials::Material>& ceil_mat, const std::shared_ptr<Materials::Material>& floor_mat, const std::shared_ptr<Materials::Material>& left_mat, const std::shared_ptr<Materials::Material>& right_mat)
		{
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p2.Z, front_mat))); // front
			objects.emplace_back(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p1.Z, back_mat)); // back
			
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p2.Y, ceil_mat))); // ceiling
			objects.emplace_back(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p1.Y, floor_mat)); // floor
			
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p2.X, right_mat))); // right
			objects.emplace_back(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p1.X, left_mat)); //left
		}

		void AddFaces()
		{
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p2.Z, nullptr))); // front
			objects.emplace_back(std::make_shared<Objects::RectangleXY>(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p1.Z, nullptr)); // back
			
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p2.Y, nullptr))); // ceiling
			objects.emplace_back(std::make_shared<Objects::RectangleXZ>(m_p1.X, m_p2.X, m_p1.Z, m_p2.Z, m_p1.Y, nullptr)); // floor
			
			objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p2.X, nullptr))); // right
			objects.emplace_back(std::make_shared<Objects::RectangleYZ>(m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z, m_p1.X, nullptr)); //left
		}

		Vector3D<double> m_p1;
		Vector3D<double> m_p2;
	};

}
