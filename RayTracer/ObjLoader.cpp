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

	std::map<std::string, std::shared_ptr<Materials::Material>> materialsMap;

	BuildMaterialsMap(materialsMap, dir);
	// now build the object out of polygons, by splitting them to triangles
	// splits it 'triangle-fan' style

	// it won't work for all polygons, works for convex ones and some concave ones

	SetTriangles(textureCoords, normals, vertices, polygons, materialsMap);

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


void ObjLoader::BuildMaterialsMap(std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::string& dirv)
{
	TexturesCache texturesCache;

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
	}
}

void ObjLoader::AddMaterialWithDiffuseTexture(std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::pair<std::string, ObjMaterial>& mat, const std::string& dir, TexturesCache& texturesCache)
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


void ObjLoader::AddMaterialNoDiffuseTexture(std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::pair<std::string, ObjMaterial>& mat, const std::string& dir, TexturesCache& texturesCache)
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


void ObjLoader::SetTriangles(const std::vector<std::pair<double, double>>& textureCoords, const std::vector<Vector3D<double>>& normals, const std::vector<Vector3D<double>>& vertices, const std::vector<std::pair<Polygon, std::string>>& polygons, std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap)
{
	const auto WhiteMaterial = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.73, 0.73, 0.73))));

	// now build the object out of polygons, by splitting them to triangles
	// splits it 'triangle-fan' style

	// it won't work for all polygons, works for convex ones and some concave ones

	for (const auto& polygonpair : polygons)
	{
		Polygon polygon = polygonpair.first;
		const std::string& matName = polygonpair.second;

		std::shared_ptr<Materials::Material> material;
		if (materialsMap.find(matName) != materialsMap.end())
			material = materialsMap[matName];
		else
			material = WhiteMaterial;

		if (!SplitPolygon(polygon, vertices, normals, textureCoords, material)) break;
	}
}

bool ObjLoader::SplitPolygon(Polygon& polygon, const std::vector<Vector3D<double>>& vertices, const std::vector<Vector3D<double>>& normals, const std::vector<std::pair<double, double>>& textureCoords, std::shared_ptr<Materials::Material>& material)
{
	int startPoint = 0;

	// if there is only one reflex interior angle, this works, if not, it's a matter of luck

	IsConcave(polygon, vertices, startPoint);


	// from here splitting begins
	// if the polygon is convex, from the first point
	// if it's concave, from a point picked above

	// a more general method would be 'ear clipping', but definitively I won't have patience for that, it's boring
	// also it's worth looking into "Optimal convex decompositions" by Bernard Chazelle and David Dobkin - a concave polygon can be split into convex ones

	const size_t indexvertex1 = std::get<0>(polygon[startPoint]);
	const long long int indextex1 = std::get<1>(polygon[startPoint]);
	const long long int indexnormal1 = std::get<2>(polygon[startPoint]);

	const size_t indexvertex2 = std::get<0>(polygon[(startPoint + 1ULL) % polygon.size()]);
	const long long int indextex2 = std::get<1>(polygon[(startPoint + 1ULL) % polygon.size()]);
	const long long int indexnormal2 = std::get<2>(polygon[(startPoint + 1ULL) % polygon.size()]);

	const size_t indexvertex3 = std::get<0>(polygon[(startPoint + 2ULL) % polygon.size()]);
	const long long int indextex3 = std::get<1>(polygon[(startPoint + 2ULL) % polygon.size()]);
	const long long int indexnormal3 = std::get<2>(polygon[(startPoint + 2ULL) % polygon.size()]);

	if (indexvertex1 >= vertices.size()) return false;
	if (indexvertex2 >= vertices.size()) return false;
	if (indexvertex3 >= vertices.size()) return false;

	if (indexnormal1 < 0 || indexnormal2 < 0 || indexnormal3 < 0)
		return SplitPolygonNoNormals(polygon, vertices, textureCoords, material);

	if (indexnormal1 >= static_cast<long long int>(normals.size())) return false;
	if (indexnormal2 >= static_cast<long long int>(normals.size())) return false;
	if (indexnormal3 >= static_cast<long long int>(normals.size())) return false;

	const Vector3D<double> firstPoint = vertices[indexvertex1];
	const long long int firstIndexTex = indextex1;
	const Vector3D<double> firstNormal = normals[indexnormal1];

	Vector3D<double> lastPoint(vertices[indexvertex3]);
	long long int lastIndexTex = indextex3;
	Vector3D<double> lastNormal(normals[indexnormal3]);

	AddTriangle(firstPoint, vertices[indexvertex2], lastPoint, firstNormal, normals[indexnormal2], lastNormal, material, textureCoords, firstIndexTex, indextex2, lastIndexTex);

	for (int i = 3; i < polygon.size(); ++i)
	{
		const size_t ind = (static_cast<size_t>(startPoint) + i) % polygon.size();

		const size_t nextIndexVertex = std::get<0>(polygon[ind]);
		if (nextIndexVertex >= vertices.size())
			break;

		const long long int nextIndexNormal = std::get<2>(polygon[ind]);
		if (nextIndexNormal >= static_cast<long long int>(normals.size()) || nextIndexNormal < 0)
			break;

		const long long int nextIndexTex = std::get<1>(polygon[ind]);

		const Vector3D<double>& nextPoint = vertices[nextIndexVertex];
		const Vector3D<double>& nextNormal = normals[nextIndexNormal];

		AddTriangle(firstPoint, lastPoint, nextPoint, firstNormal, lastNormal, nextNormal, material, textureCoords, firstIndexTex, lastIndexTex, nextIndexTex);

		lastPoint = nextPoint;
		lastIndexTex = nextIndexTex;
		lastNormal = nextNormal;
	}

	return true;
}



bool ObjLoader::SplitPolygonNoNormals(Polygon& polygon, const std::vector<Vector3D<double>>& vertices, const std::vector<std::pair<double, double>>& textureCoords, std::shared_ptr<Materials::Material>& material)
{
	int startPoint = 0;

	// if there is only one reflex interior angle, this works, if not, it's a matter of luck

	IsConcave(polygon, vertices, startPoint);


	// from here splitting begins
	// if the polygon is convex, from the first point
	// if it's concave, from a point picked above

	// a more general method would be 'ear clipping', but definitively I won't have patience for that, it's boring
	// also it's worth looking into "Optimal convex decompositions" by Bernard Chazelle and David Dobkin - a concave polygon can be split into convex ones

	const size_t indexvertex1 = std::get<0>(polygon[startPoint]);
	const long long int indextex1 = std::get<1>(polygon[startPoint]);

	const size_t indexvertex2 = std::get<0>(polygon[(startPoint + 1ULL) % polygon.size()]);
	const long long int indextex2 = std::get<1>(polygon[(startPoint + 1ULL) % polygon.size()]);

	const size_t indexvertex3 = std::get<0>(polygon[(startPoint + 2ULL) % polygon.size()]);
	const long long int indextex3 = std::get<1>(polygon[(startPoint + 2ULL) % polygon.size()]);

	if (indexvertex1 >= vertices.size()) return false;
	if (indexvertex2 >= vertices.size()) return false;
	if (indexvertex3 >= vertices.size()) return false;

	const Vector3D<double> firstPoint = vertices[indexvertex1];
	const long long int firstIndexTex = indextex1;

	Vector3D<double> lastPoint(vertices[indexvertex3]);
	long long int lastIndexTex = indextex3;

	AddTriangleNoNormals(firstPoint, vertices[indexvertex2], lastPoint, material, textureCoords, firstIndexTex, indextex2, lastIndexTex);

	for (int i = 3; i < polygon.size(); ++i)
	{
		const size_t ind = (static_cast<size_t>(startPoint) + i) % polygon.size();

		const size_t nextIndexVertex = std::get<0>(polygon[ind]);
		if (nextIndexVertex >= vertices.size())
			break;

		const long long int nextIndexTex = std::get<1>(polygon[ind]);

		const Vector3D<double>& nextPoint = vertices[nextIndexVertex];

		AddTriangleNoNormals(firstPoint, lastPoint, nextPoint, material, textureCoords, firstIndexTex, lastIndexTex, nextIndexTex);

		lastPoint = nextPoint;
		lastIndexTex = nextIndexTex;
	}

	return true;
}



void ObjLoader::AddTriangle(const Vector3D<double>& firstPoint, const Vector3D<double>& secondPoint, const Vector3D<double>& lastPoint, const Vector3D<double>& firstNormal, const Vector3D<double>& secondNormal, const Vector3D<double>& lastNormal, std::shared_ptr<Materials::Material>& material, const std::vector<std::pair<double, double>>& textureCoords, long long int firstIndexTex, long long int indextex2, long long int lastIndexTex)
{
	std::shared_ptr<Objects::Triangle> triangle = std::make_shared<Objects::Triangle>(firstPoint, secondPoint, lastPoint, firstNormal, secondNormal, lastNormal, material);

	if (firstIndexTex >= 0)
	{
		triangle->U1 = textureCoords[firstIndexTex].first;
		triangle->V1 = textureCoords[firstIndexTex].second;
	}
	else
	{
		triangle->U1 = -1;
		triangle->V1 = -1;
	}

	if (indextex2 >= 0)
	{
		triangle->U2 = textureCoords[indextex2].first;
		triangle->V2 = textureCoords[indextex2].second;
	}
	else
	{
		triangle->U2 = -1;
		triangle->V2 = -1;
	}


	if (lastIndexTex >= 0)
	{
		triangle->U3 = textureCoords[lastIndexTex].first;
		triangle->V3 = textureCoords[lastIndexTex].second;
	}
	else
	{
		triangle->U3 = -1;
		triangle->V3 = -1;
	}

	triangle->SetUseInterpolation();
	triangles.emplace_back(triangle);
}


void ObjLoader::AddTriangleNoNormals(const Vector3D<double>& firstPoint, const Vector3D<double>& secondPoint, const Vector3D<double>& lastPoint, std::shared_ptr<Materials::Material>& material, const std::vector<std::pair<double, double>>& textureCoords, long long int firstIndexTex, long long int indextex2, long long int lastIndexTex)
{
	std::shared_ptr<Objects::Triangle> triangle = std::make_shared<Objects::Triangle>(firstPoint, secondPoint, lastPoint, material);

	if (firstIndexTex >= 0)
	{
		triangle->U1 = textureCoords[firstIndexTex].first;
		triangle->V1 = textureCoords[firstIndexTex].second;
	}
	else
	{
		triangle->U1 = -1;
		triangle->V1 = -1;
	}

	if (indextex2 >= 0)
	{
		triangle->U2 = textureCoords[indextex2].first;
		triangle->V2 = textureCoords[indextex2].second;
	}
	else
	{
		triangle->U2 = -1;
		triangle->V2 = -1;
	}


	if (lastIndexTex >= 0)
	{
		triangle->U3 = textureCoords[lastIndexTex].first;
		triangle->V3 = textureCoords[lastIndexTex].second;
	}
	else
	{
		triangle->U3 = -1;
		triangle->V3 = -1;
	}

	triangle->SetUseInterpolation();
	triangles.emplace_back(triangle);
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