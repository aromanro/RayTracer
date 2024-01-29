#pragma once

#include "Vector3D.h"

class Ray
{
public:
	Ray() = default;
	Ray(const Vector3D<double>& origin, const Vector3D<double>& direction);


	inline const Vector3D<double>& getOrigin() const { return m_origin; }
	inline const Vector3D<double>& getDirection() const { return m_direction; }
	inline const Vector3D<double>& getInvDir() const { return m_invDir; }

	inline Vector3D<double> operator()(double parameter) const { return m_origin + parameter * m_direction; }

protected:
	Vector3D<double> m_origin;
	Vector3D<double> m_direction;

	Vector3D<double> m_invDir;
};

