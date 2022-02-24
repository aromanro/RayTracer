#include "ObjMaterial.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>


ObjMaterial::ObjMaterial()
{
	Clear();
}


void ObjMaterial::LoadLine(std::string& line)
{
	switch (line.at(0))
	{
	case 'K': // Ka, Kd, Ks
		LoadColor(line);
	break;
	case 'T': //Tf or Tr
	{
		std::string what = line.substr(0, 2);
		if (what == "Tf") // transmission filter
		{
			// TODO: again, for Tf is as above
			line = line.substr(3);

			// so check the prefix
			if (line.substr(0, 3) != "xyz" && line.substr(0, 8) != "spectral")
			{
				std::istringstream sstream(line);

				// TODO: implement it!
			}
			// else not implemented yet
		}
		else if (what == "Tr")
		{
			line = line.substr(3);

			std::istringstream sstream(line);
			sstream >> dissolve;
			dissolve = 1. - dissolve;
		}
	}
	break;
	case 'N': // Ns, Ni
	{
		std::string what = line.substr(0, 2);
		if (what == "Ns") // specular exponent
		{
			line = line.substr(3);

			std::istringstream sstream(line);
			sstream >> exponent;
		}
		else if (what == "Ni") // optical density
		{
			line = line.substr(3);

			std::istringstream sstream(line);
			sstream >> refractionCoeff;
		}
	}
	break;
	case 'd': //d = dissolve 
	{
		std::string what = line.substr(0, 1);
		if (what == "d") // halo factor
		{
			line = line.substr(2);

			std::istringstream sstream(line);
			sstream >> dissolve;
		}
	}
	break;
	case 'i': // illum
	{
		std::string what = line.substr(0, 5);
		if (what == "illum")
		{
			line = line.substr(6);
			int i;
			std::istringstream sstream(line);
			sstream >> i;
			illumination = ObjMaterial::Illumination(i);
		}
	}
	break;
	case 'm': // map_Ka, map_Kd, map_Ks
		LoadMap(line);
	break;
	case 'b':
	{
		std::string what = line.substr(0, 4);
		if (what == "bump") // bump, see above map_bump, it's the same thing
		{
			line = line.substr(5);
			bumpTexture = line;
		}
	}
	break;
	}
}

void ObjMaterial::LoadColor(std::string& line, Color& color)
{
	// so check the prefix
	if (line.substr(0, 3) != "xyz" && line.substr(0, 8) != "spectral")
	{
		std::istringstream sstream(line);

		sstream >> color.r;
		try
		{
			sstream >> color.g >> color.b;
		}
		catch (...)
		{
			color.g = color.b = color.r;
		}
	}
	// else not implemented yet
}


void ObjMaterial::LoadColor(std::string& line)
{
	std::string what = line.substr(0, 2);
	if (what == "Ka") // ambient reflectivity
	{
		// TODO: can actually be not ony 'Ka r g b' but also 'Ka spectral file.rfl factor' or 'Ka xyz x y z'
		line = line.substr(3);
		LoadColor(line, ambientColor);
	}
	else if (what == "Kd") // diffuse reflectivity
	{
		// TODO: for Kd is as above
		line = line.substr(3);

		LoadColor(line, diffuseColor);
	}
	else if (what == "Ks") // specular reflectivity
	{
		// TODO: for Ks is as above
		line = line.substr(3);

		LoadColor(line, specularColor);
	}
	else if (what == "Ke") // emission color
	{
		// TODO: for Ke is as above
		line = line.substr(3);

		LoadColor(line, emissionColor);
	}
}

void ObjMaterial::FixLine(std::string& line)
{
	if (line.at(0) == '\\' || line.at(0) == '/')
		line = line.substr(1);
	std::replace(line.begin(), line.end(), '/', '\\');
}


void ObjMaterial::LoadMap(std::string& line)
{
	std::string what = line.substr(0, 6);
	if (what == "map_Ka") // material ambient is multiplied by the texture value
	{
		line = line.substr(7);
		FixLine(line);
		ambientTexture = line;
	}
	else if (what == "map_Kd") // material diffuse is multiplied by the texture value
	{
		line = line.substr(7);
		FixLine(line);
		diffuseTexture = line;
	}
	else if (what == "map_Ks") // material specular is multiplied by the texture value
	{
		line = line.substr(7);
		FixLine(line);
		specularTexture = line;
	}
	else if (what == "map_Ke") // material emission color is multiplied by the texture value
	{
		line = line.substr(7);
		FixLine(line);
		glowTexture = line;
	}
	else if (what == "map_Ns") // material specular exponent is multiplied by the texture value
	{
		line = line.substr(7);
		FixLine(line);
		exponentTexture = line;
	}
	else if (what == "map_d") // material dissolve is multiplied by the texture value
	{
		line = line.substr(6);
		FixLine(line);
		dissolveTexture = line;
	}
	else if (what == "map_bump") // bump mapping, this probably should be implemented
	{
		line = line.substr(9);
		FixLine(line);
		bumpTexture = line;
	}
}
