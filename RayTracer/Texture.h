#pragma once

#include <memory>

#include "Color.h"
#include "Vector3D.h"

namespace Textures
{

	class Texture
	{
	public:
		virtual ~Texture() {}
		virtual Color Value(double u, double v, const Vector3D<double>& pos) const = 0;
	};



	class ColorTexture : public Texture
	{
	protected:
		Color m_color;
	public:
		ColorTexture(const Color& color) : m_color(color) {}
		Color Value(double u, double v, const Vector3D<double>& pos) const override { return m_color; }
	};


	class CheckerTexture : public Texture
	{
	protected:
		std::shared_ptr<Texture> m_texture1;
		std::shared_ptr<Texture> m_texture2;

		double m_scale;
	public:
		CheckerTexture(const std::shared_ptr<Texture> texture1, const std::shared_ptr<Texture> texture2, double scale = 10.) : m_texture1(texture1), m_texture2(texture2), m_scale(scale) {}

		Color Value(double u, double v, const Vector3D<double>& pos) const override
		{
			const double c = cos(m_scale * pos.X) * cos(m_scale * pos.Y) * cos(m_scale * pos.Z);

			Color val;

			if (c < 0)
			{
				val = m_texture1 ? m_texture1->Value(u, v, pos) : Color(1., 1., 1.);
			}
			else
			{
				val = m_texture2 ? m_texture2->Value(u, v, pos) : Color(1., 1., 1.);
			}

			return val;
		}
	};

}