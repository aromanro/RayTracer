#pragma once

#include "Vector3D.h"

namespace Objects
{
	// you can do triangles, quads... out of this
	class Vertex
	{
	public:
		Vertex(const Vector3D<double>& pos, const Vector3D<double>& norm) : position(pos), normal(norm) {};


		void Translate(const Vector3D<double>& t)
		{
			position += t;
		}

		void RotateAround(const Vector3D<double>& v, double angle)
		{
			position = position.RotateAround(v, angle);

			normal = normal.RotateAround(v, angle);
		}

		Vector3D<double> position;
		Vector3D<double> normal;
	};

}