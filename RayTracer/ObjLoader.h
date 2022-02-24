#pragma once

#include "Triangle.h"
#include "ObjMaterial.h"
#include "TexturesCache.h"

#include <tuple>
#include <map>

class ObjLoader
{
public:
	typedef std::vector<std::tuple<size_t, long long int, long long int>> Polygon;

	ObjLoader();
	~ObjLoader();

	static void LeftTrim(std::string& str);

	bool Load(const std::string& name, bool center = true);

	bool LoadMaterial(const std::string& name, const std::string& dir);

	static bool IsConcaveVertex(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int pointIndex, double& sine);
	static bool IsConcave(const Polygon& polygon, const std::vector<Vector3D<double>>& vertices, int& pointIndex);

	std::map<std::string, ObjMaterial> materials;
	std::vector<std::shared_ptr<Objects::Triangle>> triangles;

protected:
	static void LoadVertexInfo(std::string& line, std::vector<Vector3D<double>>& vertices, std::vector<Vector3D<double>>& normals, std::vector<std::pair<double, double>>& textureCoords, Vector3D<double>& centerCoord);
	static void LoadFace(std::string& line, const std::string& curMaterial, std::vector<std::pair<Polygon, std::string>>& polygons, const std::vector<Vector3D<double>>& vertices, const std::vector<Vector3D<double>>& normals, const std::vector<std::pair<double, double>>& textureCoords);

	void BuildMaterialsMap(std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::string& dir);
	void AddMaterialNoDiffuseTexture(std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::pair<std::string, ObjMaterial>& mat, const std::string& dir, TexturesCache& texturesCache);
	void AddMaterialWithDiffuseTexture(std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap, const std::pair<std::string, ObjMaterial>& mat, const std::string& dir, TexturesCache& texturesCache);
	void SetTriangles(const std::vector<std::pair<double, double>>& textureCoords, const std::vector<Vector3D<double>>& normals, const std::vector<Vector3D<double>>& vertices, const std::vector<std::pair<Polygon, std::string>>& polygons, std::map<std::string, std::shared_ptr<Materials::Material>>& materialsMap);
	void AddTriangle(const Vector3D<double>& firstPoint, const Vector3D<double>& secondPoint, const Vector3D<double>& lastPoint, const Vector3D<double>& firstNormal, const Vector3D<double>& secondNormal, const Vector3D<double>& lastNormal, std::shared_ptr<Materials::Material>& material, const std::vector<std::pair<double, double>>& textureCoords, long long int firstIndexTex, long long int indextex2, long long int lastIndexTex);
	bool SplitPolygon(Polygon& polygon, const std::vector<Vector3D<double>>& vertices, const std::vector<Vector3D<double>>& normals, const std::vector<std::pair<double, double>>& textureCoords, std::shared_ptr<Materials::Material>& material);
};

