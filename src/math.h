#pragma once

//! \file
//! \brief Math helpers.

#include <float.h>
#include <math.h>
#include <stdbool.h>

//! \brief Determines if two scalar values are nearly equal
//!
//! given the limitations of floating point accuracy.
__forceinline bool almost_equalf(float a, float b)
{
	float diff = fabsf(a - b);
	if (diff <= FLT_EPSILON * 1000.0f)
	{
		return true;
	}
	if (diff <= __max(fabsf(a), fabsf(b)) * FLT_EPSILON * 4.0f)
	{
		return true;
	}
	return false;
}

//! \brief Linearly interpolate between two floating point values.
//! \Return resulting value.
__forceinline float lerpf(float begin, float end, float distance)
{
	return (begin * (1.0f - distance)) + (end * distance);
}
