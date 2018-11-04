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

		for (int i = Height - 1; i >= 0; --i)
		{
			imageData[i].resize(Width);

			wxImagePixelData::Iterator rowStart = p;

			for (int j = 0; j < image.GetWidth(); ++j, ++p)
			{

				imageData[i][j].r = p.Red() / 255.;
				imageData[i][j].g = p.Green() / 255.;
				imageData[i][j].b = p.Blue() / 255.;
			}

			p = rowStart;
			if (0 != i) p.OffsetY(data, 1);
		}

		return false;
	}


	Color ImageTexture::Value(double u, double v, const Vector3D<double>& pos) const
	{
		if (imageData.empty()) return Color(0, 0, 0);

		if (flipHorizontally) u = 1. - u;
		if (flipVertically) v = 1. - v;
		if (flipAxes) std::swap(u, v);

		int i = static_cast<int>(u * Width);
		int j = static_cast<int>(v * Height - 0.001);

		if (i < 0) i = 0;
		else if (i >= Width) i = Width - 1;

		if (j < 0) j = 0;
		else if (j >= Height) j = Height - 1;

		return imageData[j][i];
	}



}