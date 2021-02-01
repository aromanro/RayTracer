#pragma once

#include "Triangle.h"

#include <tuple>
#include <map>

class ObjLoader
{
public:
	typedef std::vector<std::tuple<int, int, int>> Polygon;

	class Material
	{
	public:
		std::string name;
		
		// for now handle only r g b specification, there can be also 'spectral' or 'xyz' specs, just ignore those for now
		// if only r is specified, r=g=b
				
		Color ambientColor;  //Ka
		Color diffuseColor;  //Kd
		Color specularColor; //Ks

		Color transmColor; // Tf - transmission filter

		double exponent = 0; // Ns specular exponent - big value, stronger highlight

		std::string ambientTexture; // map_Ka
		std::string diffuseTexture; // map_Kd
		std::string specularTexture; // map_Ks


		// WARNING: there can be Ns and d textures as well, but I ignore them for now

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

		void Clear()
		{
			name.clear();
			
			ambientColor = Color(0.1, 0.1, 0.1);
			diffuseColor = Color(0.3, 0.3, 0.3);
			specularColor = Color(0.6, 0.6, 0.6);
			transmColor = Color();

			exponent = 0;

			ambientTexture.clear();
			diffuseTexture.clear();
			specularTexture.clear();

			illumination = Illumination::blinn;

			dissolve = 1;
			refractionCoeff = 0;
		}

		bool IsEmpty() const { return name.empty(); }
	};


	ObjLoader();
	~ObjLoader();

	static void LeftTrim(std::string& str);

	bool Load(const std::string& name, bool center = true);

	bool LoadMaterial(const std::string& name, const std::string& dir);

	static bool IsConcaveVertex(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int pointIndex, double& sine);
	static bool IsConcave(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int& pointIndex);

	static bool AddTriangle(int ind1, int ind2, int ind3, const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, const std::vector<Vector3D<double>>& normals, const std::vector<std::pair<double, double>>& textureCoords, std::shared_ptr<Materials::Material> material, std::vector<std::shared_ptr<Objects::Triangle>>& triangles);

	std::map<std::string, Material> materials;
	std::vector<std::shared_ptr<Objects::Triangle>> triangles;
};

