#include "ObjLoader.h"
#include "Material.h"
#include "ImageTexture.h"

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
