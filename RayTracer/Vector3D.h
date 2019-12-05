#pragma once

#include <tuple>

template<typename T> class Vector3D
{
public:
	T X;
	T Y;
	T Z;

	Vector3D();
	Vector3D(const T& v);
	template<typename O> Vector3D(const Vector3D<O>& other);
	Vector3D(const T& x, const T& y, const T& z);

	template<typename O> Vector3D& operator=(const Vector3D<O>& other);

	const Vector3D& operator+() const;
	Vector3D operator-() const;

	template<typename O> Vector3D operator+(const Vector3D<O>& other) const;
	template<typename O> Vector3D operator-(const Vector3D<O>& other) const;
	template<typename O> T operator*(const Vector3D<O>& other) const;
	template<typename O> Vector3D operator%(const Vector3D<O>& other) const;

	Vector3D operator*(T s) const;
	Vector3D operator/(T s) const;

	template<typename O> Vector3D& operator+=(const Vector3D<O>& other);
	template<typename O> Vector3D& operator-=(const Vector3D<O>& other);
	template<typename O> T operator*=(const Vector3D<O>& other);
	template<typename O> Vector3D& operator%=(const Vector3D<O>& other);

	Vector3D& operator*=(T s);
	Vector3D& operator/=(T s);

	double Length() const;
	Vector3D Normalize() const;

	template<typename O, typename A> Vector3D RotateAround(const Vector3D<O>& other, A angle) const;
	template<typename O, typename A> Vector3D RotateTowards(const Vector3D<O>& other, A angle) const;

	double getTheta() const
	{
		return acos(Z / Length());
	}

	double getPhi() const
	{
		return atan(Y / X);
	}
};

template<typename T> Vector3D<T> operator*(T o, const Vector3D<T>& t) { return t*o; }
template<typename T> bool operator==(const Vector3D<T>& f, const Vector3D<T>& t) { return f.X == t.X && f.Y == t.Y && f.Z == t.Z; }

template<typename T> bool operator<(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
{
	return std::tie(lhs.X, lhs.Y, lhs.Z) < std::tie(rhs.X, rhs.Y, rhs.Z);
}

#ifndef _VECTOR_3D_IMPL
#include "Vector3D.inl"
#endif