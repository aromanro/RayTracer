#include "ObjLoader.h"
#include "Material.h"
#include "ImageTexture.h"

#include <fstream>
#include <sstream>
#include <cctype>


#define _MATH_DEFINES_DEFINED

#include <filesystem>


ObjLoader::ObjLoader()
{
}


ObjLoader::~ObjLoader()
{
}


bool ObjLoader::Load(const std::string& name, bool center)
{
	std::filesystem::path dirName(name);

	std::ifstream infile(name);

	if (!infile) return false;

	const std::string dir = dirName.remove_filename().string();

	std::vector<std::pair<double, double>> textureCoords;
	std::vector<Vector3D<double>> normals;
	std::vector<Vector3D<double>> vertices;

	std::vector<std::pair<Polygon, std::string>> polygons; // will be converted to triangles in the end

	triangles.clear();
	materials.clear();

	std::string line;

	Vector3D<double> centerCoord;
	std::string curMaterial;

	while (std::getline(infile, line))
	{
		if (line.length() >= 2)
		{
			switch (line.at(0))
			{
			case 'v': // vertex info
				LoadVertexInfo(line, vertices, normals, textureCoords, centerCoord);
				break;

			case 'f': // face - can be a triangle or in general, some polygon
			{
				line = line.substr(2);

				LoadFace(line, curMaterial, polygons, vertices, normals, textureCoords);
			}
			break;

			// can be more, materials, textures...
			case 's':
				// TODO: Should I do something with this? It's 'smooth' or something like that
				break;
			case 'm':
				if (line.substr(0, 6) == "mtllib")
				{
					line = line.substr(7);

					LoadMaterial(line, dir);
				}
				break;

			case 'u':
				if (line.substr(0, 6) == "usemtl")
				{
					line = line.substr(7);

					curMaterial = line;
				}

				break;
			} // switch			
		} // line length > 2
	} // while

	infile.close();

	if (center) // center the object in origin
	{
		centerCoord /= static_cast<double>(vertices.size());
		for (auto& vertex : vertices)
			vertex -= centerCoord;
	}


	// from here start building the object from the info loaded from the .obj file

	// first, create the materials

	std::unordered_map<std::string, std::shared_ptr<Materials::Material>> materialsMap;
	std::unordered_map<std::string, std::shared_ptr<Textures::Texture>> normalsMap;

	BuildMaterialsAndNormalsMaps(materialsMap, normalsMap, dir);
	// now build the object out of polygons, by splitting them to triangles
	// splits it 'triangle-fan' style

	// it won't work for all polygons, works for convex ones and some concave ones

	SetTriangles(textureCoords, normals, vertices, polygons, materialsMap, normalsMap);

	return true;
}


void ObjLoader::LoadFace(std::string& line, const std::string& curMaterial, std::vector<std::pair<Polygon, std::string>>& polygons, const std::vector<Vector3D<double>>& vertices, const std::vector<Vector3D<double>>& normals, const std::vector<std::pair<double, double>>& textureCoords)
{
	std::istringstream sstream(line);
	std::string token;

	Polygon polygon;

	while (std::getline(sstream, token, ' '))
	{
		// a 'token' contains info about the vertex
		// index/index/index, where index starts from 1 and is
		// first number: index for the vertex
		// second: index for texture coordinate (can miss)
		// third: index for the normal (can miss?)

		std::istringstream tokenstream(token);


		long long int indexvertex = 0;
		long long int indextex = 0;
		long long int indexnormal = 0;

		std::string val;
		int cnt = 0;
		while (std::getline(tokenstream, val, '/'))
		{
			if (0 == cnt)
			{
				indexvertex = std::stoi(val);
				// indices may be negative, in that case it indexes from last position backwards
				if (indexvertex < 0)
					indexvertex = vertices.size() + indexvertex + 1; // +1 because there will be decremented later
			}
			else if (1 == cnt)
			{
				if (!val.empty())
				{
					indextex = std::stoi(val);

					// indices may be negative, in that case it indexes from last position backwards
					if (indextex < 0)
						indextex = textureCoords.size() + indextex + 1; // +1 because there will be decremented later
				}
			}
			else
			{
				if (!val.empty())
				{
					indexnormal = std::stoi(val);
					// indices may be negative, in that case it indexes from last position backwards
					if (indexnormal < 0)
						indexnormal = normals.size() + indexnormal + 1; // +1 because there will be decremented later
				}

				break;
			}

			++cnt;
		}

		--indexvertex;
		--indextex;
		--indexnormal;

		if (indexvertex < 0)
			break;

		polygon.emplace_back(std::make_tuple(indexvertex, indextex, indexnormal));
	}

	if (polygon.size() > 2) polygons.emplace_back(std::make_pair(polygon, curMaterial));
}


void ObjLoader::LoadVertexInfo(std::string& line, std::vector<Vector3D<double>>& vertices, std::vector<Vector3D<double>>& normals, std::vector<std::pair<double, double>>& textureCoords, Vector3D<double>& centerCoord)
{
	if (line.at(1) == 't') // texture coordinates
	{
		line = line.substr(3);

		std::istringstream sstream(line);

		double u = 0;
		double v = 0;

		sstream >> u >> v;

		// for tilling
		u -= floor(u);
		v -= floor(v);

		textureCoords.emplace_back(std::make_pair(u, v));
	}
	else if (line.at(1) == 'n') // vertex normal
	{
		line = line.substr(3);

		Vector3D<double> normal;

		std::istringstream sstream(line);
		sstream >> normal.X >> normal.Y >> normal.Z;

		normals.emplace_back(normal);
	}
	else if (line.at(1) == ' ' || line.at(1) == '\t') // the actual vertex
	{
		line = line.substr(2);

		Vector3D<double> vertex;

		std::istringstream sstream(line);
		sstream >> vertex.X >> vertex.Y >> vertex.Z;

		vertices.emplace_back(vertex);

		centerCoord += vertex;
	}
}


void ObjLoader::BuildMaterialsAndNormalsMaps(std::unordered_map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, std::unordered_map<std::string, std::shared_ptr<Textures::Texture>>& normalsMap, const std::string& dirv)
{
	TexturesCache texturesCache;
	Color bcolor;

	std::string dir = dirv;
	if (!dir.empty() && dir.at(dir.size() - 1) != '\\' && dir.at(dir.size() - 1) != '/')
		dir += "\\";

	for (const auto& mat : materials)
	{
		if (mat.second.diffuseTexture.empty())
		{
			AddMaterialNoDiffuseTexture(materialsMap, mat, dir, texturesCache);
		}
		else
		{
			AddMaterialWithDiffuseTexture(materialsMap, mat, dir, texturesCache);
		}

		if (!mat.second.bumpTexture.empty())
		{
			auto tex = texturesCache.Get(dir + mat.second.bumpTexture, bcolor);

			normalsMap[mat.first] = tex;
		}
	}
}

void ObjLoader::AddMaterialWithDiffuseTexture(std::unordered_map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::pair<std::string, ObjMaterial>& mat, const std::string& dir, TexturesCache& texturesCache)
{
	const std::string tname = dir + mat.second.diffuseTexture;
	auto tex = texturesCache.Get(tname, mat.second.diffuseColor);

	if (mat.second.IsTransparent()) materialsMap[mat.first] = std::make_shared<Materials::Dielectric>(mat.second.refractionCoeff <= 1. ? 1.5 : mat.second.refractionCoeff, tex);
	else
	{
		if (mat.second.IsSpecular())
		{
			std::shared_ptr<Textures::Texture> specTexture;


			if (mat.second.specularTexture.empty())
				specTexture = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(mat.second.specularColor));
			else
			{
				const std::string tname = dir + mat.second.specularTexture;
				specTexture = texturesCache.Get(tname, mat.second.specularColor);
			}

			const double exponent = mat.second.exponent/*/ 10.*/;

			if (mat.second.exponentTexture.empty())
				materialsMap[mat.first] = std::make_shared<Materials::AnisotropicPhong>(exponent, exponent, tex, specTexture);
			else
			{
				const std::string tname = dir + mat.second.exponentTexture;
				std::shared_ptr<Textures::Texture> expTexture = texturesCache.Get(tname, Color());

				materialsMap[mat.first] = std::make_shared<Materials::AnisotropicPhong>(exponent, exponent, tex, specTexture, expTexture);
			}
		}
		else materialsMap[mat.first] = std::make_shared<Materials::Lambertian>(tex);
	}
}


void ObjLoader::AddMaterialNoDiffuseTexture(std::unordered_map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::pair<std::string, ObjMaterial>& mat, const std::string& dir, TexturesCache& texturesCache)
{
	auto tex = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>((mat.second.IsTransparent() && mat.second.diffuseColor.VeryAbsorbing()) ? mat.second.specularColor : mat.second.diffuseColor));

	if (mat.second.IsTransparent()) materialsMap[mat.first] = std::make_shared<Materials::Dielectric>(mat.second.refractionCoeff <= 1. ? 1.5 : mat.second.refractionCoeff, tex);
	else
	{
		if (mat.second.IsSpecular())
		{
			std::shared_ptr<Textures::Texture> specTexture;

			if (mat.second.specularTexture.empty())
				specTexture = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(mat.second.specularColor));
			else
			{
				const std::string tname = dir + mat.second.specularTexture;
				specTexture = texturesCache.Get(tname, mat.second.specularColor);
			}

			const double exponent = mat.second.exponent /*/ 10.*/;

			if (mat.second.exponentTexture.empty())
				materialsMap[mat.first] = std::make_shared<Materials::AnisotropicPhong>(exponent, exponent, tex, specTexture);
			else
			{
				const std::string tname = dir + mat.second.exponentTexture;
				std::shared_ptr<Textures::Texture> expTexture = texturesCache.Get(tname, Color());

				materialsMap[mat.first] = std::make_shared<Materials::AnisotropicPhong>(exponent, exponent, tex, specTexture, expTexture);
			}
		}
		else materialsMap[mat.first] = std::make_shared<Materials::Lambertian>(tex);
	}
}




bool ObjLoader::IsConcaveVertex(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int cp, double& sine)
{
	sine = 0;
	const size_t pp = (cp == 0 ? polygon.size() : cp) - 1;
	const size_t np = (cp == polygon.size() - 1) ? 0 : cp + 1;

	const size_t indpp = std::get<0>(polygon[pp]);
	if (indpp < 0 || indpp >= vertices.size()) return false;

	const size_t indcp = std::get<0>(polygon[cp]);
	if (indcp < 0 || indcp >= vertices.size()) return false;

	const size_t indnp = std::get<0>(polygon[np]);
	if (indnp < 0 || indnp >= vertices.size()) return false;

	const Vector3D<double> prevPoint = vertices[indpp];
	const Vector3D<double> curPoint = vertices[indcp];
	const Vector3D<double> nextPoint = vertices[indnp];

	const Vector3D<double> edge1 = (prevPoint - curPoint).Normalize();
	const Vector3D<double> edge2 = (nextPoint - curPoint).Normalize();

	sine = (edge1 % edge2).Length();

	return sine < 0;
}

bool ObjLoader::IsConcave(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int& pointIndex)
{
	// this walks around the polygon trying to find if it's concave, if it is, tries to find the best vertex to start splitting from

	if (polygon.size() > 3)
	{
		double worstSine = 0;

		//#define CHECK_WINDING 1
#ifdef CHECK_WINDING
		int concave = 0;
		for (int cp = pointIndex; cp < polygon.size(); ++cp)
		{
			double sine;
			if (IsConcaveVertex(polygon, vertices, cp, sine))
			{
				++concave;
				if (concave > polygon.size() / 2)
					break;
			}
		}
#endif


		for (int cp = pointIndex; cp < polygon.size(); ++cp)
		{
			double sine;

#ifdef CHECK_WINDING
			if (concave > polygon.size() / 2)
			{
				if (IsConcaveVertex(polygon, vertices, cp, sine)) continue;

				if (sine > worstSine)
				{
					worstSine = sine;
					pointIndex = cp;
					return true; // just pick up the first one
				}
			}
			else
			{
#endif
				if (!IsConcaveVertex(polygon, vertices, cp, sine)) continue;

				if (sine < worstSine)
				{
					worstSine = sine;
					pointIndex = cp;
					return true; // just pick up the first one
				}
#ifdef CHECK_WINDING
			}
#endif
		}

		//if (worstSine < 0) return true;
	}

	return false;
}


void ObjLoader::LeftTrim(std::string& line)
{
	line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int c) { return !std::isspace(c); }));
}


bool ObjLoader::LoadMaterial(const std::string& name, const std::string& dir)
{
	std::ifstream infile(dir + name);

	if (!infile) return false;

	ObjMaterial mat;

	std::string line;
	while (std::getline(infile, line))
	{
		LeftTrim(line);

		if (line.length() >= 2)
		{
			if (line.substr(0, 6) == "newmtl")
			{
				if (!mat.IsEmpty())
				{
					mat.FixColors();
					// save the old filled one
					materials[mat.name] = mat;
				}

				mat.Clear();

				line = line.substr(7);
				mat.name = line;
			}
			else
				mat.LoadLine(line);
		}
	}

	if (!mat.IsEmpty() && materials.find(mat.name) == materials.end())
	{
		mat.FixColors();
		materials[mat.name] = mat;	
	}

	return true;
}