#include "geometry.h"

using namespace pbrt;
int main()
{
	Vector3f vec1(1.93234234f, 2.1f, 3.5f);
	Vector3f vec2(1.93234234f, 2.1f, 3.5f);
	std::cout << vec1 << std::endl;
    std::cout << vec2 - vec1 << std::endl;
    vec2 -= vec1;
    std::cout << vec2 << std::endl;
    vec2 += vec1;
    std::cout << vec1 << std::endl;
    vec2 *= 3.f;
    std::cout << vec2 << std::endl;
    std::cout << 3 * vec2 << std::endl;
}