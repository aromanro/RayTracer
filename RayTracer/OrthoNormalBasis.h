#pragma once

#include "Vector3D.h"

class OrthoNormalBasis
{
public:
	OrthoNormalBasis() {};

	// this will have passed the normal to the surface
	OrthoNormalBasis(const Vector3D<double>& v) { BuildFrom(v); };
	// the normal and the incident ray vector
	OrthoNormalBasis(const Vector3D<double>& n, const Vector3D<double>& i) { BuildFrom(n, i); };

	~OrthoNormalBasis() {};


	inline Vector3D<double> operator[](int i) const { return basis[i]; }

	// u and v are two orthogonal vectors that are tangent to the surface
	inline Vector3D<double> u() const { return basis[0]; }
	inline Vector3D<double> v() const { return basis[1]; }
	inline Vector3D<double> w() const { return basis[2]; } // the normal to the surface, really

	inline Vector3D<double> Normal() const { return w(); }

	inline Vector3D<double> LocalToGlobal(double X, double Y, double Z) const { return X * basis[0] + Y * basis[1] + Z * basis[2]; }
	inline Vector3D<double> LocalToGlobal(const Vector3D<double>& vect) const { return vect.X * basis[0] + vect.Y * basis[1] + vect.Z * basis[2]; }

	inline void BuildFrom(const Vector3D<double>& v) 
	{
		//basis[2] = v.Normalize(); // don't normalize, pass it as normalized already, so avoid useless computations
		basis[2] = v;
		const bool useY = abs(basis[2].X) > 0.9;

		if (useY) basis[1] = Vector3D<double>(0, 1, 0);
		else basis[1] = Vector3D<double>(1, 0, 0);

		basis[1] = (basis[1] % basis[2]).Normalize();
		basis[0] = basis[1] % basis[2]; // x = y % z
	}

	inline void BuildFrom(const Vector3D<double>& n, const Vector3D<double>& i)
	{
		const double cosine = abs(n * i);

		if (cosine > 0.99999999) BuildFrom(n);
		else
		{
			basis[2] = n; // z
			basis[0] = (i % n).Normalize(); // x
			basis[1] = basis[2] % basis[0]; // x = z % x
		}
	}

protected:
	Vector3D<double> basis[3];
};


