#include "ObjLoader.h"
#include "Material.h"
#include "ImageTexture.h"

#include <fstream>
#include <sstream>


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

	typedef std::vector<std::tuple<int, int, int>> Polygon;
	std::vector<std::pair<Polygon, std::string>> polygons; // will be converted to triangles in the end

	triangles.clear();

	std::string line;

	Vector3D<double> centerCoord;
	
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
				else // the actual vertex
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
							}

							++cnt;
						}

						--indexvertex;
						--indextex;
						--indexnormal;

						if (indexvertex < 0 || indexnormal < 0) break;

						polygon.emplace_back(std::make_tuple(indexvertex, indextex, indexnormal));
					}

					if (polygon.size() > 2) polygons.emplace_back(std::make_pair(polygon, curMaterial));
				}
				break;

			// can be more, materials, textures...
			case 'm':
				if (line.substr(0, 7) == "mtllib ")
				{
					std::string name = line.substr(7);
					
					LoadMaterial(name, dir);
				}
				break;

			case 'u':
				if (line.substr(0, 7) == "usemtl ")
					curMaterial = line.substr(7);
				break;

			}
		}

	}

	infile.close();

	if (center)
	{
		centerCoord /= vertices.size();
		for (auto& vertex : vertices)
			vertex -= centerCoord;

	}

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

	for (const auto& polygonpair : polygons)
	{
		const auto& polygon = polygonpair.first;
		const std::string& matName = polygonpair.second;

		const int indexvertex1 = std::get<0>(polygon[0]);
		const int indextex1 = std::get<1>(polygon[0]);
		const int indexnormal1 = std::get<2>(polygon[0]);

		const int indexvertex2 = std::get<0>(polygon[1]);
		const int indextex2 = std::get<1>(polygon[1]);
		const int indexnormal2 = std::get<2>(polygon[1]);

		const int indexvertex3 = std::get<0>(polygon[2]);
		const int indextex3 = std::get<1>(polygon[2]);
		const int indexnormal3 = std::get<2>(polygon[2]);

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

		

		std::shared_ptr<Materials::Material> material;
		if (materialsMap.find(matName) != materialsMap.end())
			material = materialsMap[matName];
		else
			material = WhiteMaterial;

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
		
		// the rest of the points - this assumes it's a convex polygons, it splits it in a 'triangle fan' style
		// it won't work with all polygons out there in obj files
		// so better use obj files that have only triangles or at most quads
		// a better splitting cold be implemented, but for now I won't bother

		// as an easy solution for a simple case, you may try each new vertex and check for already composed triangles, testing if it's not inside any of them
		// if it is inside, drop every triangle you made for the current polygon and restart splitting starting from the current vertex
		// this should work if only one vertex is the reason of concavity
		// another way to find that vertex: just 'walk' around the polygon, checking the angle


	
		for (int i = 3; i < polygon.size(); ++i)
		{
			const int nextIndexVertex = std::get<0>(polygon[i]);
			if (nextIndexVertex < 0 || nextIndexVertex >= vertices.size()) 
				break;

			const int nextIndexNormal = std::get<2>(polygon[i]);
			if (nextIndexNormal < 0 || nextIndexNormal >= normals.size()) 
				break;


			const int nextIndexTex = std::get<1>(polygon[i]);

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


bool ObjLoader::LoadMaterial(const std::string& name, const std::string& dir)
{
	std::ifstream infile(dir + name);

	if (!infile) return false;

	Material mat;

	std::string line;
	while (std::getline(infile, line))
	{		
		if (line.length() >= 2)
		{
			while((line.at(0) == ' ' || line.at(0) == '\t') && line.length() >=2)
				line.erase(line.begin());
			
			switch (line.at(0))
			{
			case 'K': // Ka, Kd, Ks
				{
					std::string what = line.substr(0, 3);
					if (what == "Ka ")
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
					else if (what == "Kd ")
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
					std::string what = line.substr(0, 3);
					if (what == "Tf ")
					{
						line = line.substr(3);
						std::istringstream sstream(line);

						// TODO: implement it!
					}
					else if (what == "Tr ")
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
					std::string what = line.substr(0, 3);
					if (what == "Ns ")
					{	
						line = line.substr(3);
						std::istringstream sstream(line);
						sstream >> mat.exponent;
					}
					else if (what == "Ni ")
					{
						line = line.substr(3);
						std::istringstream sstream(line);
						sstream >> mat.refractionCoeff;
					}
				}
				break;
			case 'd': //d
				{
					std::string what = line.substr(0, 2);
					if (what == "d ")
					{
						line = line.substr(2);
						std::istringstream sstream(line);
						sstream >> mat.dissolve;
					}
				}
				break;
			case 'i': // illum
				{
					std::string what = line.substr(0, 6);
					if (what == "illum ")
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
					std::string what = line.substr(0, 7);
					if (what == "map_Ka ")
					{	
						line = line.substr(7);
						mat.ambientTexture = line;
					}
					else if (what == "map_Kd ")
					{
						line = line.substr(7);
						mat.diffuseTexture = line;
					}
					else if (what == "map_Ks ")
					{
						line = line.substr(7);
						mat.specularTexture = line;
					}
				}
				break;
			case 'n':
				if (line.substr(0, 7) == "newmtl ")
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


	if (!mat.IsEmpty())
		materials[mat.name] = mat;

	return true;
}