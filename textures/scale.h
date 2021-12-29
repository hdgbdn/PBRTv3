#ifndef PBRT_TEXTURES_SCALE_H
#define PBRT_TEXTURES_SCALE_H

#include "pbrt.h"
#include "texture.h"

namespace pbrt
{
	template <typename T1, typename T2>
	class ScaleTexture : public Texture<T2>
	{
	public:
		ScaleTexture(const std::shared_ptr<Texture<T1>>& tex1,
		             const std::shared_ptr<Texture<T2>>& tex2)
			: tex1(tex1), tex2(tex2)
		{
		}
		T2 Evaluate(const SurfaceInteraction& si) const {
			return tex1->Evaluate(si) * tex2->Evaluate(si);
		}
	private:
		std::shared_ptr<Texture<T1>> tex1;
		std::shared_ptr<Texture<T2>> tex2;
	};
}

#endif