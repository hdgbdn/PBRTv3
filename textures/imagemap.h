#ifndef PBRT_TEXTURES_IMAGEMAP_H
#define PBRT_TEXTURES_IMAGEMAP_H

#include "pbrt.h"
#include "texture.h"
#include "mipmap.h"
#include <map>
#include "imageio.h"

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
		             ImageWrap wrapMode, float scale, bool gamma)
			: mapping(std::move(mapping)), mipmap(GetTexture(filename, doTrilinear, maxAniso,
			                                                 wrapMode, scale, gamma)) { }

		Treturn Evaluate(const SurfaceInteraction& si) const
		{
			Vector2f dstdx, dstdy;
			Point2f st = mapping->Map(si, &dstdx, &dstdy);
			Tmemory mem = mipmap->Lookup(st, dstdx, dstdy);
			Treturn ret;
			convertOut(mem, &ret);
			return ret;
		} 

		static MIPMap<Tmemory>* GetTexture(const std::string& filename,
		           bool doTrilinear, Float maxAniso, ImageWrap wrap, Float scale,
		           bool gamma)
		{
			TexInfo texInfo(filename, doTrilinear, maxAniso, wrap, scale, gamma);
			if (textures.find(texInfo) != textures.end()) return textures[texInfo].get();
			Point2i resolution;
			std::unique_ptr<RGBSpectrum[]> texels = ReadImage(filename, &resolution);
			MIPMap<Tmemory>* mipmap = nullptr;
			if(texels)
			{
				std::unique_ptr<Tmemory[]> convertedTexels(new Tmemory[resolution.x * resolution.y]);
				for (int i = 0; i < resolution.x * resolution.y; ++i)
					convertIn(texels[i], &convertedTexels[i], scale, gamma);
				mipmap = new MIPMap<Tmemory>(resolution, convertedTexels.get(), doTrilinear, maxAniso, wrap);
			}
			else
			{
				Tmemory oneVal = scale;
				mipmap = new MIPMap<Tmemory>(Point2i(1, 1), &oneVal);
			}
			textures.reset(mipmap);
		}
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
}

#endif