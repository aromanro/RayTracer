#include "ImageTexture.h"

#define wxNEEDS_DECL_BEFORE_TEMPLATE

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include <wx/rawbmp.h>


namespace Textures
{

	ImageTexture::ImageTexture()
		: Width(0), Height(0)
	{
	}

	ImageTexture::ImageTexture(const std::string& name)
	{
		Load(name);
	}




	ImageTexture::~ImageTexture()
	{
	}


	bool ImageTexture::Load(const std::string& name)
	{
		imageData.clear();

		wxImage image(name);

		if (!image.IsOk()) return false;

		wxImagePixelData data(image);
		if (!data) return false;

		wxImagePixelData::Iterator p(data);
		p.Offset(data, 0, 0);


		Height = image.GetHeight();
		Width = image.GetWidth();

		imageData.resize(Height);

		for (int line = Height - 1; line >= 0; --line)
		{
			imageData[line].resize(Width);

			wxImagePixelData::Iterator rowStart = p;

			for (int col = 0; col < image.GetWidth(); ++col, ++p)
			{

				imageData[line][col].r = p.Red() / 255.;
				imageData[line][col].g = p.Green() / 255.;
				imageData[line][col].b = p.Blue() / 255.;
			}

			p = rowStart;
			if (0 != line) p.OffsetY(data, 1);
		}

		return false;
	}


	Color ImageTexture::Value(double u, double v, const Vector3D<double>& pos) const
	{
		if (imageData.empty()) return Color(0, 0, 0);

		if (flipHorizontally) u = 1. - u;
		if (flipVertically) v = 1. - v;
		if (flipAxes) std::swap(u, v);

		int col = static_cast<int>(u * Width);
		int line = static_cast<int>(v * Height - 0.001);

		if (col < 0) col = 0;
		else if (col >= Width) col = Width - 1;

		if (line < 0) line = 0;
		else if (line >= Height) line = Height - 1;

		return imageData[line][col];
	}



}