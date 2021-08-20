#pragma once

#include "ImageTexture.h"

namespace Textures
{

	class NormalsTexture : public ImageTexture
	{
	public:
		NormalsTexture();
		NormalsTexture(const std::string& name);
		virtual ~NormalsTexture();

		virtual bool Load(const std::string& name) override;
	};

}

