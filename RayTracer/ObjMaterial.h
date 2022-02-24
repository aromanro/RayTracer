#pragma once

#include <string>
#include "Color.h"

class ObjMaterial
{
public:
	ObjMaterial();

	std::string name;

	// for now handle only r g b specification, there can be also 'spectral' or 'xyz' specs, just ignore those for now
	// if only r is specified, r=g=b

	Color ambientColor;  //Ka
	Color diffuseColor;  //Kd
	Color specularColor; //Ks
	Color emissionColor; //Ke

	Color transmColor; // Tf - transmission filter

	double exponent = 0; // Ns specular exponent - big value, stronger highlight

	// NOTE: The ambient from the 'obj' is not used

	std::string ambientTexture; // map_Ka
	std::string diffuseTexture; // map_Kd
	std::string specularTexture; // map_Ks

	std::string exponentTexture; // map_Ns
	std::string dissolveTexture; // map_d
	std::string bumpTexture; // map_bump or bump

	std::string glowTexture; // map_Ke

	enum class Illumination {
		constant = 0, // Kd color
		diffuse, // Lambertian shading
		blinn,    // actually Blinn-Phong diffuse and specular combined
		reflective, // Blinn-Phong plus reflection
		transparent, // Blinn-Phong plus transparency
		fresnelReflection, // Blinn-Phong plus Fresnel reflection
		transparentNoReflection,
		transparentReflection,
		reflectionNoRayTrace,
		transparentNoRayTrace,
		castShadows
	};

	Illumination illumination = Illumination::blinn; // illum

	double dissolve = 1.; // d - dissolve factor, 1 fully opaque - for OpenGL it would be alpha
	double refractionCoeff = 0.; // Ni = optical density, glass = 1.5, 1. no refraction

	bool IsSpecular() const { return exponent > 0 && (illumination == Illumination::blinn || illumination == Illumination::reflective) && (!specularColor.VeryAbsorbing() || !specularTexture.empty()); }


	bool IsTransparent() const
	{
		return illumination == Illumination::transparent || illumination == Illumination::transparentNoReflection || illumination == Illumination::transparentReflection || illumination == Illumination::transparentNoRayTrace
			|| dissolve < 0.99;
	}

	void FixColors()
	{
		if (ambientColor.TotalAbsorbant())
		{
			ambientColor = diffuseColor;
			if (ambientTexture.empty())
				ambientTexture = diffuseTexture;
		}

		if (diffuseColor.TotalAbsorbant())
		{
			diffuseColor = ambientColor;

			if (diffuseTexture.empty())
				diffuseTexture = ambientTexture;
		}
	}

	void Clear()
	{
		name.clear();

		illumination = Illumination::blinn;

		ambientColor = Color(0.1, 0.1, 0.1);
		diffuseColor = Color(0.3, 0.3, 0.3);
		specularColor = Color(0.6, 0.6, 0.6);
		transmColor = Color();
		emissionColor = Color();

		ambientTexture.clear();
		diffuseTexture.clear();
		specularTexture.clear();

		exponentTexture.clear();
		dissolveTexture.clear();
		bumpTexture.clear();
		glowTexture.clear();


		exponent = 0;
		dissolve = 1;
		refractionCoeff = 0;
	}

	bool IsEmpty() const { return name.empty(); }

	void LoadLine(std::string& line);

protected:
	void LoadColor(std::string& line);
	void LoadColor(std::string& line, Color& color);
	void LoadMap(std::string& line);
	void FixLine(std::string& line);
};

