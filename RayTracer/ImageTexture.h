#pragma once

#include <string>
#include <vector>

#include "Texture.h"
#include "Color.h"

namespace Textures
{

	class ImageTexture : public Texture
	{
	public:
		ImageTexture();
		ImageTexture(const std::string& name);
		virtual ~ImageTexture();

		virtual bool Load(const std::string& name);

		virtual Color Value(double u, double v, const Vector3D<double>& pos) const override;

		int Width;
		int Height;

		std::vector<std::vector<Color>> imageData;

		bool flipHorizontally = false;
		bool flipVertically = false;
		bool flipAxes = false;
	};

}