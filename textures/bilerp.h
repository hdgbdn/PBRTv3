#ifndef PBRT_TEXTURES_BILERP_H
#define PBRT_TEXTURES_BILERP_H

#include "core/pbrt.h"
#include "core/texture.h"

namespace pbrt
{
	template<typename T>
	class BilerpTexture : public Texture<T>
	{
	public:
		BilerpTexture(std::unique_ptr<TextureMapping2D> mapping,
		              const T& v00, const T& v01, const T& v10, const T& v11)
			: mapping(std::move(mapping)), v00(v00), v01(v01), v10(v10),
			  v11(v11)
		{
		}
		T Evaluate(const SurfaceInteraction& si) const {
			Vector2f dstdx, dstdy;
			Point2f st = mapping->Map(si, &dstdx, &dstdy);
			return (1 - st[0]) * (1 - st[1]) * v00 + (1 - st[0]) * (st[1]) * v01 +
				(st[0]) * (1 - st[1]) * v10 + (st[0]) * (st[1]) * v11;
		}
	private:
		std::unique_ptr<TextureMapping2D> mapping;
		const T v00, v01, v10, v11;
	};
}

#endif