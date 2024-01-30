#include "TexturesCache.h"
#include "ImageTexture.h"


std::shared_ptr<Textures::Texture> TexturesCache::Get(const std::string& tname, const Color& color)
{
	std::tuple<std::string, double, double, double> index = std::make_tuple(tname, color.r, color.g, color.b);

	if (texturesCache.find(index) == texturesCache.end())
	{
		auto theTexture = std::make_shared<Textures::ImageTexture>(tname);
		
		if (!color.TotalAbsorbant() && !color.TotalReflective())
			theTexture->MultiplyWith(color);

		texturesCache.try_emplace(index, theTexture);
	}

	return texturesCache[index];
}
