#pragma once

// very similar with vector but we don't need so many operations
// there are some other functions that are color-related here, too

class Color
{
public:
	Color(double rr = 0, double gg = 0, double bb = 0);

	double r;
	double g;
	double b;

	// ease up some things to allow combining colors, attenuating/amplifying them and so on

	Color& operator*=(double s) { r *= s; g *= s; b *= s; return *this; };
	Color& operator/=(double s) { r /= s; g /= s; b /= s; return *this; };
	Color& operator+=(double s) { r += s; g += s; b += s; return *this; };
	Color& operator-=(double s) { r -= s; g -= s; b -= s; return *this; };

	Color& operator+=(const Color& o) { r += o.r; g += o.g; b += o.b; return *this; };
	Color& operator-=(const Color& o) { r -= o.r; g -= o.g; b -= o.b; return *this; };
	Color& operator*=(const Color& o) { r *= o.r; g *= o.g; b *= o.b; return *this; };
	Color& operator/=(const Color& o) { r /= o.r; g /= o.g; b /= o.b; return *this; };

	Color operator*(double s) const { return Color(r * s, g * s, b * s); };
	Color operator/(double s) const { return Color(r / s, g / s, b / s); };
	Color operator-(double s) const { return Color(r - s, g - s, b - s); };
	Color operator+(double s) const { return Color(r + s, g + s, b + s); };

	Color operator+(const Color& o) const { return Color(r + o.r, g + o.g, b + o.b); };
	Color operator-(const Color& o) const { return Color(r + o.r, g + o.g, b + o.b); };
	Color operator*(const Color& o) const { return Color(r * o.r, g * o.g, b * o.b); };
	Color operator/(const Color& o) const { return Color(r / o.r, g / o.g, b / o.b); };


	void Clamp() 
	{
		ClampVal(r);
		ClampVal(g);
		ClampVal(b);
	}

	double Max() const
	{
		return r > g && r > b ? r : g > b ? g : b;
	}

	bool TotalReflective() const
	{
		return 1. == r && 1. == g && 1. == b;
	}

	bool TotalAbsorbant() const
	{
		return 0. >= r && 0. >= g && 0. >= b;
	}

	bool VeryAbsorbing() const
	{
		return r <= 0.01 && g <= 0.01 && b <= 0.01;
	}

	bool Emitter() const
	{
		return r > 1.01 || g > 1.01 || b > 1.01;
	}

protected:
	static void ClampVal(double &val)
	{
		if (val < 0) val = 0;
		else if (val > 1) val = 1;
	}
};


Color operator*(double t, const Color& o);
