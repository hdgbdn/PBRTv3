#ifndef PBRT_TEXTURES_IMAGEMAP_H
#define PBRT_TEXTURES_IMAGEMAP_H

#include "core/pbrt.h"
#include "core/texture.h"
#include "core/mipmap.h"
#include "core/imageio.h"
#include <map>

namespace pbrt
{
	struct TexInfo {
		TexInfo(const std::string& f, bool dt, float ma, ImageWrap wm, float sc,
			bool gamma)
			: filename(f),
			doTrilinear(dt),
			maxAniso(ma),
			wrapMode(wm),
			scale(sc),
			gamma(gamma) {}
		std::string filename;
		bool doTrilinear;
		float maxAniso;
		ImageWrap wrapMode;
		float scale;
		bool gamma;
		bool operator<(const TexInfo& t2) const {
			if (filename != t2.filename) return filename < t2.filename;
			if (doTrilinear != t2.doTrilinear) return doTrilinear < t2.doTrilinear;
			if (maxAniso != t2.maxAniso) return maxAniso < t2.maxAniso;
			if (scale != t2.scale) return scale < t2.scale;
			if (gamma != t2.gamma) return !gamma;
			return wrapMode < t2.wrapMode;
		}
	};
	template <typename Tmemory, typename Treturn>
	class ImageTexture : public Texture<Treturn>
	{
	public:
		ImageTexture(std::unique_ptr<TextureMapping2D> mapping,
		             const std::string& filename, bool doTrilinear, float maxAniso,
		             ImageWrap wrapMode, float scale, bool gamma);

		Treturn Evaluate(const SurfaceInteraction& si) const;

		static MIPMap<Tmemory>* GetTexture(const std::string& filename,
		           bool doTrilinear, float maxAniso, ImageWrap wrap, float scale,
		           bool gamma);
	private:
		std::unique_ptr<TextureMapping2D> mapping;
		MIPMap<Tmemory>* mipmap;
		static std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>> textures;
		static void convertIn(const RGBSpectrum& from, RGBSpectrum* to, float scale, bool gamma)
		{
			for (int i = 0; i < RGBSpectrum::nSamples; ++i)
				(*to)[i] = scale * (gamma ? InverseGammaCorrect(from[i])
					: from[i]);
		}
		static void convertIn(const RGBSpectrum& from, float* to,
			float scale, bool gamma) {
			*to = scale * (gamma ? InverseGammaCorrect(from.y())
				: from.y());
		}
		static void convertOut(const RGBSpectrum& from, Spectrum* to) {
			float rgb[3];
			from.ToRGB(rgb);
			*to = Spectrum::FromRGB(rgb);
		}
		static void convertOut(float from, float* to) {
			*to = from;
		}
	};

    template <typename Tmemory, typename Treturn>
    std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>>
            ImageTexture<Tmemory, Treturn>::textures;

    ImageTexture<float, float> *CreateImageFloatTexture(const Transform &tex2world,
                                                        const TextureParams &tp);

    ImageTexture<RGBSpectrum, Spectrum> *CreateImageSpectrumTexture(
            const Transform &tex2world, const TextureParams &tp);
}

#endif