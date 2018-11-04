#pragma once

#include "Vector3D.h"
#include "Ray.h"
#include "Random.h"

// for a more complex camera look into the SolarSystem or EventMolecularDynamics projects
// this is very basic, it's going to be static, ray tracing is slow
// this project will generate one image only
// so the camera is not going to rotate, translate and so on

class Camera
{
public:
	Camera(const Vector3D<double>& origin = Vector3D<double>(13., 2., 3.), const Vector3D<double>& target = Vector3D<double>(0., 0., 0.), const Vector3D<double>& upv = Vector3D<double>(0., 1., 0.), double ang = 40, double aspect = 1080. / 1920., double aperture = 0, double focusDist = 1.);
	~Camera();

	// x, y - fractions of viewport size
	inline Ray getRay(double x, double y, Random& random) const
	{		
		if (0. == lensRadius) 
			return Ray(eyePos, orig + x * viewportWidthVec + y * viewportHeightVec);

		const Vector3D<double> rvec = lensRadius * random.getRandomInUnitSphere();
		const Vector3D<double> off = rvec.X * right + rvec.Y * up;

		return Ray(eyePos + off, orig + x * viewportWidthVec + y * viewportHeightVec - off);
	}


protected:
	Vector3D<double> eyePos;
	Vector3D<double> lookAt;
	Vector3D<double> up;

	// helpers
	Vector3D<double> right;
	Vector3D<double> lowerLeftCorner;

	// viewport info
	Vector3D<double> viewportWidthVec;
	Vector3D<double> viewportHeightVec;

	// lens info
	double lensRadius;

	Vector3D<double> orig;
};

