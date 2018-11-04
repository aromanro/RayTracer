#define _VECTOR_3D_IMPL

#include "Vector3D.h"

#include <cmath>


template<typename T> Vector3D<T>::Vector3D() 
	: X(0), Y(0), Z(0) 
{
}


template<typename T> Vector3D<T>::Vector3D(const T& v)
	: X(v), Y(v), Z(v)
{
}

template<typename T> template<typename O> Vector3D<T>::Vector3D(const Vector3D<O>& other)
	: X(other.X), Y(other.Y), Z(other.Z)
{
}

template<typename T> Vector3D<T>::Vector3D(const T& x, const T& y, const T& z)
	: X(x), Y(y), Z(z)
{
}


template<typename T> template<typename O> Vector3D<T>& Vector3D<T>::operator=(const Vector3D<O>& other) 
{
	X = other.X;
	Y = other.Y;
	Z = other.Z;

	return *this;
}


template<typename T> const Vector3D<T>& Vector3D<T>::operator+() const
{
	return *this;
}

template<typename T> Vector3D<T> Vector3D<T>::operator-() const
{
	return Vector3D<T>(-X, -Y, -Z);
}

template<typename T> template<typename O> Vector3D<T> Vector3D<T>::operator+(const Vector3D<O>& other) const
{
	return Vector3D<T>(X + other.X, Y + other.Y, Z + other.Z);
}

template<typename T> template<typename O> Vector3D<T> Vector3D<T>::operator-(const Vector3D<O>& other) const
{
	return Vector3D<T>(X - other.X, Y - other.Y, Z - other.Z);
}

template<typename T> template<typename O> T Vector3D<T>::operator*(const Vector3D<O>& other) const
{
	return X*other.X + Y*other.Y + Z*other.Z;
}

template<typename T> template<typename O> Vector3D<T> Vector3D<T>::operator%(const Vector3D<O>& other) const
{
	return Vector3D<T>(Y * other.Z - Z * other.Y, Z * other.X - X * other.Z, X * other.Y - Y * other.X);
}

template<typename T> Vector3D<T> Vector3D<T>::operator*(T s) const
{
	return Vector3D<T>(X*s, Y*s, Z*s);
}

template<typename T> Vector3D<T> Vector3D<T>::operator/(T s) const
{
	return Vector3D<T>(X/s, Y/s, Z/s);
}

template<typename T> template<typename O> Vector3D<T>& Vector3D<T>::operator+=(const Vector3D<O>& other)
{
	return *this = *this + other;
}

template<typename T> template<typename O> Vector3D<T>& Vector3D<T>::operator-=(const Vector3D<O>& other)
{
	return *this = *this - other;
}

template<typename T> template<typename O> T Vector3D<T>::operator*=(const Vector3D<O>& other)
{
	return *this = *this * other;
}

template<typename T> template<typename O> Vector3D<T>& Vector3D<T>::operator%=(const Vector3D<O>& other)
{
	return *this = *this % other;
}

template<typename T> Vector3D<T>& Vector3D<T>::operator*=(T s)
{
	return *this = *this * s;
}

template<typename T> Vector3D<T>& Vector3D<T>::operator/=(T s)
{
	return *this = *this / s;
}


template<typename T> double Vector3D<T>::Length() const
{
	const double w = *this * *this;

	return sqrt(w);
}

template<typename T> Vector3D<T> Vector3D<T>::Normalize() const
{
	const T len = Length();

	if (len != 0)
		return *this / len;

	return *this;
}

template<typename T> template<typename O, typename A> Vector3D<T> Vector3D<T>::RotateAround(const Vector3D<O>& other, A angle) const
{
	Vector3D<T> result;
	const Vector3D<O> u = other.Normalize();

	const T sinAngle = sin(angle);
	const T cosAngle = cos(angle);
	const T oneMinus = 1. - cosAngle;

	result.X = (cosAngle + oneMinus * u.X * u.X)       * X + (oneMinus * u.X * u.Y - u.Z * sinAngle) * Y + (oneMinus * u.X * u.Z + u.Y * sinAngle) * Z;
	result.Y = (oneMinus * u.X * u.Y + u.Z * sinAngle) * X + (cosAngle + oneMinus * u.Y * u.Y)       * Y + (oneMinus * u.Y * u.Z - u.X * sinAngle) * Z;
	result.Z = (oneMinus * u.X * u.Z - u.Y * sinAngle) * X + (oneMinus * u.Y * u.Z + u.X * sinAngle) * Y + (cosAngle + oneMinus * u.Z * u.Z)       * Z;

	return result;
}

template<typename T> template<typename O, typename A> Vector3D<T> Vector3D<T>::RotateTowards(const Vector3D<O>& other, A angle) const
{
	const Vector3D<T> a = this->operator%(other);

	return RotateAround(a, angle);
}
