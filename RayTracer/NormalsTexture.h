#pragma once

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

	class NormalsTexture : public ImageTexture
	{
	public:
		NormalsTexture();
		NormalsTexture(const std::string& name);
		virtual ~NormalsTexture();

		bool Load(const std::string& name) override;

		double bumpParam;

	protected:
		double GetPixelValue(const wxImagePixelData& data, wxImagePixelData::Iterator& p, int x, int y) const;
	};

}

