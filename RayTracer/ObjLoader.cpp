#include "ObjLoader.h"
#include "Material.h"
#include "ImageTexture.h"

#include <fstream>
#include <sstream>
#include <cctype>


#define _MATH_DEFINES_DEFINED

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

#include <wx/filename.h>





ObjLoader::ObjLoader()
{
}


ObjLoader::~ObjLoader()
{
}


bool ObjLoader::Load(const std::string& name, bool center)
{
	wxFileName dirName(name);

	std::ifstream infile(name);

	if (!infile) return false;

	const std::string dir = std::string(dirName.GetPathWithSep().c_str());

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

				break;

			case 'f': // face - can be a triangle or in general, some polygon
			{
				line = line.substr(2);

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


					int indexvertex = 0;
					int indextex = 0;
					int indexnormal = 0;
					std::string val;
					int cnt = 0;
					while (std::getline(tokenstream, val, '/'))
					{
						if (0 == cnt)
						{
							indexvertex = std::stoi(val);
							// indices may be negative, in that case it indexes from current position backwards
							if (indexvertex < 0) indexvertex = vertices.size() + indexvertex + 1; // +1 because there will be decremented later
						}
						else if (1 == cnt)
						{
							if (!val.empty())
							{
								indextex = std::stoi(val);

								// indices may be negative, in that case it indexes from current position backwards
								if (indextex < 0) indextex = textureCoords.size() + indextex + 1; // +1 because there will be decremented later									
							}
						}
						else
						{
							if (!val.empty())
							{
								indexnormal = std::stoi(val);
								// indices may be negative, in that case it indexes from current position backwards
								if (indexnormal < 0) indexnormal = normals.size() + indexnormal + 1; // +1 because there will be decremented later
							}

							break;
						}

						++cnt;
					}

					--indexvertex;
					--indextex;
					--indexnormal;

					if (indexvertex < 0 || indexnormal < 0)
						break;

					polygon.emplace_back(std::make_tuple(indexvertex, indextex, indexnormal));
				}

				if (polygon.size() > 2) polygons.emplace_back(std::make_pair(polygon, curMaterial));
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
		centerCoord /= vertices.size();
		for (auto& vertex : vertices)
			vertex -= centerCoord;
	}


	// from here start building the object from the info loaded from the .obj file


	// first, create the materials

	const auto WhiteMaterial = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.73, 0.73, 0.73))));

	std::map<std::string, std::shared_ptr<Materials::Material>> materialsMap;

	bool addSlash = false;
	if (!dir.empty() && dir.at(dir.size() - 1) != '\\' && dir.at(dir.size() - 1) != '/')
		addSlash = true;


	for (const auto& mat : materials)
	{
		if (mat.second.diffuseTexture.empty())
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
						bool addSlashNeed = addSlash && mat.second.specularTexture.at(0) != '\\' && mat.second.specularTexture.at(0) != '/';
						const std::string tname = dir + (addSlashNeed ? "\\" : "") + mat.second.specularTexture;
						specTexture = std::make_shared<Textures::ImageTexture>(tname);
					}

					const double exponent = mat.second.exponent / 10.;
					materialsMap[mat.first] = std::make_shared<Materials::AnisotropicPhong>(exponent, exponent, tex, specTexture);
				}
				else materialsMap[mat.first] = std::make_shared<Materials::Lambertian>(tex);
			}
		}
		else
		{
			bool addSlashNeed = addSlash && mat.second.diffuseTexture.at(0) != '\\' && mat.second.diffuseTexture.at(0) != '/';
			const std::string tname = dir + (addSlashNeed ? "\\" : "") + mat.second.diffuseTexture;
			auto imTex = std::make_shared<Textures::ImageTexture>(tname);

			auto tex = std::dynamic_pointer_cast<Textures::Texture>(imTex);

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
						addSlashNeed = addSlash && mat.second.specularTexture.at(0) != '\\' && mat.second.specularTexture.at(0) != '/';
						const std::string tname = dir + (addSlashNeed ? "\\" : "") + mat.second.specularTexture;
						specTexture = std::make_shared<Textures::ImageTexture>(tname);
					}

					const double exponent = mat.second.exponent / 10.;
					materialsMap[mat.first] = std::make_shared<Materials::AnisotropicPhong>(exponent, exponent, tex, specTexture);
				}
				else materialsMap[mat.first] = std::make_shared<Materials::Lambertian>(tex);
			}
		}
	}

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


		int startPoint = 0;

		// if there is only one reflex interior angle, this works, if not, it's a matter of luck

		IsConcave(polygon, vertices, startPoint);

		// from here splitting begins
		// if the polygon is convex, from the first point
		// if it's concave, from a point picked above

		// a more general method would be 'ear clipping', but definitively I won't have patience for that, it's boring
		// also it's worth looking into "Optimal convex decompositions" by Bernard Chazelle and David Dobkin - a concave polygon can be split into convex ones

		const int indexvertex1 = std::get<0>(polygon[startPoint]);
		const int indextex1 = std::get<1>(polygon[startPoint]);
		const int indexnormal1 = std::get<2>(polygon[startPoint]);

		const int indexvertex2 = std::get<0>(polygon[(startPoint + 1) % polygon.size()]);
		const int indextex2 = std::get<1>(polygon[(startPoint + 1) % polygon.size()]);
		const int indexnormal2 = std::get<2>(polygon[(startPoint + 1) % polygon.size()]);

		const int indexvertex3 = std::get<0>(polygon[(startPoint + 2) % polygon.size()]);
		const int indextex3 = std::get<1>(polygon[(startPoint + 2) % polygon.size()]);
		const int indexnormal3 = std::get<2>(polygon[(startPoint + 2) % polygon.size()]);

		if (indexvertex1 < 0 || indexvertex1 >= vertices.size()) break;
		if (indexvertex2 < 0 || indexvertex2 >= vertices.size()) break;
		if (indexvertex3 < 0 || indexvertex3 >= vertices.size()) break;

		if (indexnormal1 < 0 || indexnormal1 >= normals.size()) break;
		if (indexnormal2 < 0 || indexnormal2 >= normals.size()) break;
		if (indexnormal3 < 0 || indexnormal3 >= normals.size()) break;

		const Vector3D<double> firstPoint = vertices[indexvertex1];
		const int firstIndexTex = indextex1;
		const Vector3D<double> firstNormal = normals[indexnormal1];

		Vector3D<double> lastPoint(vertices[indexvertex3]);
		int lastIndexTex = indextex3;
		Vector3D<double> lastNormal(normals[indexnormal3]);


		std::shared_ptr<Objects::Triangle> triangle = std::make_shared<Objects::Triangle>(firstPoint, vertices[indexvertex2], lastPoint, firstNormal, normals[indexnormal2], lastNormal, material);

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


		for (int i = 3; i < polygon.size(); ++i)
		{
			const int ind = (startPoint + i) % polygon.size();

			const int nextIndexVertex = std::get<0>(polygon[ind]);
			if (nextIndexVertex < 0 || nextIndexVertex >= vertices.size())
				break;

			const int nextIndexNormal = std::get<2>(polygon[ind]);
			if (nextIndexNormal < 0 || nextIndexNormal >= normals.size())
				break;

			const int nextIndexTex = std::get<1>(polygon[ind]);

			const Vector3D<double>& nextPoint = vertices[nextIndexVertex];
			const Vector3D<double>& nextNormal = normals[nextIndexNormal];

			triangle = std::make_shared<Objects::Triangle>(firstPoint, lastPoint, nextPoint, firstNormal, lastNormal, nextNormal, material);

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

			if (lastIndexTex >= 0)
			{
				triangle->U2 = textureCoords[lastIndexTex].first;
				triangle->V2 = textureCoords[lastIndexTex].second;
			}
			else
			{
				triangle->U2 = -1;
				triangle->V2 = -1;
			}


			if (nextIndexTex >= 0)
			{
				triangle->U3 = textureCoords[nextIndexTex].first;
				triangle->V3 = textureCoords[nextIndexTex].second;
			}
			else
			{
				triangle->U3 = -1;
				triangle->V3 = -1;
			}

			triangle->SetUseInterpolation();

			triangles.emplace_back(triangle);

			lastPoint = nextPoint;
			lastIndexTex = nextIndexTex;
			lastNormal = nextNormal;
		}
	}

	return true;
}



bool ObjLoader::IsConcaveVertex(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int cp, double& cosine)
{
	cosine = 0;
	const int pp = (cp == 0 ? polygon.size() : cp) - 1;
	const int np = (cp == polygon.size() - 1) ? 0 : cp + 1;

	const int indpp = std::get<0>(polygon[pp]);
	if (indpp < 0 || indpp >= vertices.size()) return false;

	const int indcp = std::get<0>(polygon[cp]);
	if (indcp < 0 || indcp >= vertices.size()) return false;

	const int indnp = std::get<0>(polygon[np]);
	if (indnp < 0 || indnp >= vertices.size()) return false;

	const Vector3D<double> prevPoint = vertices[indpp];
	const Vector3D<double> curPoint = vertices[indcp];
	const Vector3D<double> nextPoint = vertices[indnp];

	const Vector3D<double> edge1 = (curPoint - prevPoint).Normalize();
	const Vector3D<double> edge2 = (nextPoint - curPoint).Normalize();

	cosine = edge1 * edge2;

	return cosine < 0;
}

bool ObjLoader::IsConcave(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int& pointIndex)
{
	// this walks around the polygon trying to find if it's concave, if it is, tries to find the best vertex to start splitting from

	if (polygon.size() > 3)
	{
		double worstCosine = 0;

		for (int cp = pointIndex; cp < polygon.size(); ++cp)
		{
			double cosine;
			if (!IsConcaveVertex(polygon, vertices, cp, cosine)) continue;

			if (cosine < worstCosine)
			{
				worstCosine = cosine;
				pointIndex = cp;
				return true; // just pick up the first one
			}
		}

		//if (worstCosine < 0) return true;
	}

	return false;
}

bool ObjLoader::AddTriangle(int ind1, int ind2, int ind3, const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, const std::vector<Vector3D<double>>& normals, const std::vector<std::pair<double, double>>& textureCoords, std::shared_ptr<Materials::Material> material, std::vector<std::shared_ptr<Objects::Triangle>>& triangles)
{
	const int indexvertex1 = std::get<0>(polygon[ind1]);
	const int indextex1 = std::get<1>(polygon[ind1]);
	const int indexnormal1 = std::get<2>(polygon[ind1]);

	const int indexvertex2 = std::get<0>(polygon[ind2]);
	const int indextex2 = std::get<1>(polygon[ind2]);
	const int indexnormal2 = std::get<2>(polygon[ind2]);

	const int indexvertex3 = std::get<0>(polygon[ind3]);
	const int indextex3 = std::get<1>(polygon[ind3]);
	const int indexnormal3 = std::get<2>(polygon[ind3]);

	if (indexvertex1 < 0 || indexvertex1 >= vertices.size()) return false;
	if (indexvertex2 < 0 || indexvertex2 >= vertices.size()) return false;
	if (indexvertex3 < 0 || indexvertex3 >= vertices.size()) return false;

	if (indexnormal1 < 0 || indexnormal1 >= normals.size()) return false;
	if (indexnormal2 < 0 || indexnormal2 >= normals.size()) return false;
	if (indexnormal3 < 0 || indexnormal3 >= normals.size()) return false;

	const Vector3D<double> firstPoint = vertices[indexvertex1];
	const int firstIndexTex = indextex1;
	const Vector3D<double> firstNormal = normals[indexnormal1];

	Vector3D<double> lastPoint(vertices[indexvertex3]);
	int lastIndexTex = indextex3;
	Vector3D<double> lastNormal(normals[indexnormal3]);

	std::shared_ptr<Objects::Triangle> triangle = std::make_shared<Objects::Triangle>(firstPoint, vertices[indexvertex2], lastPoint, firstNormal, normals[indexnormal2], lastNormal, material);

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

	return true;
}



void ObjLoader::LeftTrim(std::string& line)
{
	line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int c) { return !std::isspace(c); }));
}


bool ObjLoader::LoadMaterial(const std::string& name, const std::string& dir)
{
	std::ifstream infile(dir + name);

	if (!infile) return false;

	Material mat;

	std::string line;
	while (std::getline(infile, line))
	{
		LeftTrim(line);

		if (line.length() >= 2)
		{
			switch (line.at(0))
			{
			case 'K': // Ka, Kd, Ks
			{
				std::string what = line.substr(0, 2);
				if (what == "Ka")
				{
					line = line.substr(3);
					std::istringstream sstream(line);

					sstream >> mat.ambientColor.r;
					try
					{
						sstream >> mat.ambientColor.g >> mat.ambientColor.b;
					}
					catch (...)
					{
						mat.ambientColor.g = mat.ambientColor.b = mat.ambientColor.r;
					}
				}
				else if (what == "Kd")
				{
					line = line.substr(3);
					std::istringstream sstream(line);
					sstream >> mat.diffuseColor.r;
					try
					{
						sstream >> mat.diffuseColor.g >> mat.diffuseColor.b;
					}
					catch (...)
					{
						mat.diffuseColor.g = mat.diffuseColor.b = mat.diffuseColor.r;
					}
				}
				else if (what == "Ks")
				{
					line = line.substr(3);
					std::istringstream sstream(line);
					sstream >> mat.specularColor.r;
					try
					{
						sstream >> mat.specularColor.g >> mat.specularColor.b;
					}
					catch (...)
					{
						mat.specularColor.g = mat.specularColor.b = mat.specularColor.r;
					}
				}
			}
			break;
			case 'T': //Tf or Tr
			{
				std::string what = line.substr(0, 2);
				if (what == "Tf")
				{
					line = line.substr(3);

					std::istringstream sstream(line);

					// TODO: implement it!
				}
				else if (what == "Tr")
				{
					line = line.substr(3);

					std::istringstream sstream(line);
					sstream >> mat.dissolve;
					mat.dissolve = 1. - mat.dissolve;
				}
			}
			break;
			case 'N': // Ns, Ni
			{
				std::string what = line.substr(0, 2);
				if (what == "Ns")
				{
					line = line.substr(3);

					std::istringstream sstream(line);
					sstream >> mat.exponent;
				}
				else if (what == "Ni")
				{
					line = line.substr(3);

					std::istringstream sstream(line);
					sstream >> mat.refractionCoeff;
				}
			}
			break;
			case 'd': //d
			{
				std::string what = line.substr(0, 1);
				if (what == "d")
				{
					line = line.substr(2);

					std::istringstream sstream(line);
					sstream >> mat.dissolve;
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
					mat.illumination = Material::Illumination(i);
				}
			}
			break;
			case 'm': // map_Ka, map_Kd, map_Ks
			{
				std::string what = line.substr(0, 6);
				if (what == "map_Ka")
				{
					line = line.substr(7);
					mat.ambientTexture = line;
				}
				else if (what == "map_Kd")
				{
					line = line.substr(7);
					mat.diffuseTexture = line;
				}
				else if (what == "map_Ks")
				{
					line = line.substr(7);
					mat.specularTexture = line;
				}
			}
			break;
			case 'n':
				if (line.substr(0, 6) == "newmtl")
				{
					if (!mat.IsEmpty())
					{
						// save the old filled one
						materials[mat.name] = mat;
					}

					mat.Clear();

					line = line.substr(7);
					mat.name = line;
				}
				break;
			}
		}
	}


	if (!mat.IsEmpty() && materials.find(mat.name) != materials.end())
		materials[mat.name] = mat;

	return true;
}