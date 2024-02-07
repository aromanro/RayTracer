#pragma once

#include <memory>

#include "Color.h"
#include "Vector3D.h"

namespace Textures
{

	class Texture
	{
	public:
		virtual ~Texture() = default;
		virtual Color Value(double u, double v, const Vector3D<double>& pos) const = 0;
	};



	class ColorTexture : public Texture
	{
	public:
		explicit ColorTexture(const Color& color) : m_color(color) {}
		Color Value(double u, double v, const Vector3D<double>& pos) const override { return m_color; }

		const Color& GetColor() const { return m_color; }

	private:
		Color m_color;
	};


	class CheckerTexture : public Texture
	{
	public:
		CheckerTexture(const std::shared_ptr<Texture> texture1, const std::shared_ptr<Texture> texture2, double scale = 10.) : m_texture1(texture1), m_texture2(texture2), m_scale(scale) {}

		Color Value(double u, double v, const Vector3D<double>& pos) const override
		{
			Color val;

			if (cos(m_scale * pos.X) * cos(m_scale * pos.Y) * cos(m_scale * pos.Z) < 0)
			{
				val = m_texture1 ? m_texture1->Value(u, v, pos) : Color(1., 1., 1.);
			}
			else
			{
				val = m_texture2 ? m_texture2->Value(u, v, pos) : Color(1., 1., 1.);
			}

			return val;
		}

	private:
		std::shared_ptr<Texture> m_texture1;
		std::shared_ptr<Texture> m_texture2;

		double m_scale;
	};

}