#ifndef PBRT_TEXTURES_MIX_H
#define PBRT_TEXTURES_MIX_H

#include "core/pbrt.h"
#include "core/texture.h"

namespace pbrt
{
	template<typename T>
	class MixTexture : public Texture<T>
	{
	public:
		MixTexture(const std::shared_ptr<Texture<T>>& tex1, 
			const std::shared_ptr<Texture<T>>& tex2, 
			const std::shared_ptr<Texture<float>>& amount)
				: tex1(tex1), tex2(tex2), amount(amount) { }
		T Evaluate(const SurfaceInteraction* si) const
		{
			T t1 = tex1->Evaluate(si), t2 = tex2->Evaluate(si);
			float amt = amount->Evaluate(*si);
			return (1 - amt) * t1 + amt * t2;
		}
	private:
		std::shared_ptr<Texture<T>> tex1, tex2;
		std::shared_ptr<Texture<float>> amount;
	};
}

#endif