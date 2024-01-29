#pragma once

#include <string>
#include <memory>
#include <map>

#include "Texture.h"
#include "Color.h"

class TexturesCache
{
public:
	std::shared_ptr<Textures::Texture> Get(const std::string& tname, const Color& color);

	void Clear() { texturesCache.clear(); }

private:
	// the texture can be multiplied by a color, whence the r, g, b components also used for indexing here
	std::map<std::tuple<std::string, double, double, double>, std::shared_ptr<Textures::Texture>> texturesCache;
};

