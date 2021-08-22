#include "NormalsTexture.h"


namespace Textures
{

	NormalsTexture::NormalsTexture()
		: ImageTexture(), bumpParam(2.)
	{
	}

	NormalsTexture::NormalsTexture(const std::string& name)
		: ImageTexture(), bumpParam(2.)
	{
		Load(name);
	}




	NormalsTexture::~NormalsTexture()
	{
	}


	bool NormalsTexture::Load(const std::string& name)
	{
		imageData.clear();

		wxImage image(name);

		if (!image.IsOk()) return false;

		unsigned int format = wxBMP_24BPP;
		if (image.HasOption(wxIMAGE_OPTION_BMP_FORMAT))
			format = image.GetOptionInt(wxIMAGE_OPTION_BMP_FORMAT);

		bool heightMap = false;
		if (format == wxBMP_8BPP_GREY)
		{
			// it's a height map, convert accordingly
			heightMap = true;
		}

		wxImagePixelData data(image);
		if (!data) return false;

		wxImagePixelData::Iterator p(data);
		p.Offset(data, 0, 0);

		Height = image.GetHeight();
		Width = image.GetWidth();

		imageData.resize(Height);

		if (heightMap)
		{
			// convert to a normal map using a Sobel operator, as in SolarSystem project

			for (int y = 0; y < data.GetHeight(); ++y)
			{
				for (int x = 0; x < data.GetWidth(); ++x)
				{
					const double top = GetPixelValue(data, p, x, y + 1);
					const double bottom = GetPixelValue(data, p, x, y - 1);
					const double left = GetPixelValue(data, p, x - 1, y);
					const double right = GetPixelValue(data, p, x + 1, y);

					const double topLeft = GetPixelValue(data, p, x - 1, y + 1);
					const double bottomLeft = GetPixelValue(data, p, x - 1, y - 1);
					const double topRight = GetPixelValue(data, p, x + 1, y + 1);
					const double bottomRight = GetPixelValue(data, p, x + 1, y - 1);

					// Sobel
					const double dX = topRight - topLeft + 2. * (right - left) + bottomRight - bottomLeft;
					const double dY = bottomLeft - topLeft + 2. * (bottom - top) + bottomRight - topRight;

					const double dZ = bumpParam; // make it smaller to increase the slope 

					Vector3D<double> v(-dX, -dY, dZ);
					v = v.Normalize();

					// now convert to RGB
					const double R = 0.5 * (1. + v.X);
					const double G = 0.5 * (1. + v.Y);
					const double B = 0.5 * (1. + v.Y);

					imageData[y][x].r = R;
					imageData[y][x].g = G;
					imageData[y][x].b = B;
				}
			}
		}
		else
		{
			for (int line = Height - 1; line >= 0; --line)
			{
				imageData[line].resize(Width);

				const wxImagePixelData::Iterator rowStartSave = p;

				for (int col = 0; col < Width; ++col, ++p)
				{
					imageData[line][col].r = p.Red() / 255.;
					imageData[line][col].g = p.Green() / 255.;
					imageData[line][col].b = p.Blue() / 255.;
				}

				p = rowStartSave;
				if (0 != line) p.OffsetY(data, 1);
			}
		}

		return true;
	}


	double NormalsTexture::GetPixelValue(const wxImagePixelData& data, wxImagePixelData::Iterator& p, int x, int y) const
	{
		const int width = data.GetWidth();
		const int height = data.GetHeight();

		if (x < 0) x += width;
		else if (x >= width) x -= width;

		if (y < 0) y += height;
		else if (y >= height) y -= height;

		p.Offset(data, x, y);

		return (static_cast<double>(p.Red()) + static_cast<double>(p.Green()) + static_cast<double>(p.Blue())) / (3. * 255.);

		return 0.;
	}

}
