#include "Ray.h"



Ray::Ray(const Vector3D<double>& origin, const Vector3D<double>& direction)
	: m_origin(origin), m_direction(direction.Normalize())
{
	m_invDir.X = 1. / m_direction.X;
	m_invDir.Y = 1. / m_direction.Y;
	m_invDir.Z = 1. / m_direction.Z;
}



