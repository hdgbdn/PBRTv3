#include "geometry.h"

using namespace pbrt;
int main()
{
	Point3f p1(1.3f, 1.2f, 3.9f);
	Point3f p2(1.2f, 9.3f, 5.8f);
	Vector3f v1(2.4f, 3.6f, 9.3f);
	auto p3 = p2 + v1;
	auto p4 = Lerp(.4f, p1, p2);
	std::cout << p4;
}