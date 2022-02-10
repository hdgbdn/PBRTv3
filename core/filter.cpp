#include "filter.h"

namespace pbrt
{
	Filter::Filter(const Vector2f& radius)
		: radius(radius), invRadius(Vector2f(1 / radius.x, 1 / radius.y))
	{
	}
}
