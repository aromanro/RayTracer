#include "NormalsTexture.h"


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

	NormalsTexture::NormalsTexture()
		: ImageTexture()
	{
	}

	NormalsTexture::NormalsTexture(const std::string& name)
		: ImageTexture()
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
}
