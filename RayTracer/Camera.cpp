#include "Camera.h"

#define _USE_MATH_DEFINES
#include <math.h>

Camera::Camera(const Vector3D<double>& origin, const Vector3D<double>& target, const Vector3D<double>& upv, double angle, double aspect, double aperture, double focusDist)
	: eyePos(origin), lookAt(target), up(upv), lensRadius(aperture / 2.)
{	
	const double rangle = angle * M_PI/180.;

	const double viewportWidth = 2 * tan(rangle/2);
	const double viewportHeight = aspect * viewportWidth;

	const Vector3D<double> lookVec = (eyePos - lookAt).Normalize();

	const Vector3D<double> u = up%lookVec;
	const Vector3D<double> v = lookVec%u;

	up = v.Normalize();
	right = u.Normalize();
	lowerLeftCorner = eyePos - viewportWidth / 2. * focusDist * right - viewportHeight / 2. * focusDist * up - focusDist * lookVec;

	viewportWidthVec = viewportWidth * focusDist * right;
	viewportHeightVec = viewportHeight * focusDist * up;

	orig = lowerLeftCorner - eyePos;
}

