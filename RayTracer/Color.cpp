#include "Color.h"



Color::Color(double rr, double gg, double bb)
 : r(rr), g(gg), b(bb)
{
}

Color operator*(double t, const Color& o) 
{ 
	return o * t; 
}