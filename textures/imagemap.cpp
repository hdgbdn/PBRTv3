#include "imagemap.h"
#include "paramset.h"
#include "fileutil.h"

namespace pbrt
{
    template<typename Tmemory, typename Treturn>
    ImageTexture<Tmemory, Treturn>::ImageTexture(std::unique_ptr<TextureMapping2D> mapping, const std::string &filename,
                                                 bool doTrilinear, float maxAniso, ImageWrap wrapMode, float scale,
                                                 bool gamma)
            : mapping(std::move(mapping)), mipmap(GetTexture(filename, doTrilinear, maxAniso,
                                                             wrapMode, scale, gamma)) { }

    template<typename Tmemory, typename Treturn>
    Treturn ImageTexture<Tmemory, Treturn>::Evaluate(const SurfaceInteraction &si) const
    {
        Vector2f dstdx, dstdy;
        Point2f st = mapping->Map(si, &dstdx, &dstdy);
        Tmemory mem = mipmap->Lookup(st, dstdx, dstdy);
        Treturn ret;
        convertOut(mem, &ret);
        return ret;
    }

    template<typename Tmemory, typename Treturn>
    MIPMap<Tmemory> *
    ImageTexture<Tmemory, Treturn>::GetTexture(const std::string &filename, bool doTrilinear, float maxAniso,
                                               ImageWrap wrap, float scale, bool gamma)
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
        textures[texInfo].reset(mipmap);
        return mipmap;
    }

    ImageTexture<float, float> *CreateImageFloatTexture(const Transform &tex2world, const TextureParams &tp)
    {
        std::unique_ptr<TextureMapping2D> map;
        std::string type = tp.FindString("mapping", "uv");
        if (type == "uv")
        {
            float su = tp.FindFloat("uscale", 1.);
            float sv = tp.FindFloat("vscale", 1.);
            float du = tp.FindFloat("udelta", 0.);
            float dv = tp.FindFloat("vdelta", 0.);
            map = std::make_unique<UVMapping2D>(su, sv, du, dv);
        }
        else if (type == "spherical")
            map = std::make_unique<SphericalMapping2D>(Inverse(tex2world));
        else if (type == "cylindrical")
            map = std::make_unique<CylindricalMapping2D>(Inverse(tex2world));
        else
        {
            Error("2D texture mapping \"%s\" unknown", type.c_str());
            map = std::make_unique<UVMapping2D>();
        }

        float maxAniso = tp.FindFloat("maxanisotropy", 8.f);
        bool trilerp = tp.FindBool("trilinear", false);
        std::string wrap = tp.FindString("wrap", "repeat");
        ImageWrap wrapMode = ImageWrap::Repeat;
        if (wrap == "black")
            wrapMode = ImageWrap::Black;
        else if (wrap == "clamp")
            wrapMode = ImageWrap::Clamp;
        float scale = tp.FindFloat("scale", 1.f);
        std::string filename = tp.FindFilename("filename");
        bool gamma = tp.FindBool("gamma", HasExtension(filename, ".tga") ||
                                          HasExtension(filename, ".png"));
        return new ImageTexture<float, float>(
                std::move(map), filename, trilerp, maxAniso, wrapMode, scale, gamma
        );
    }

    ImageTexture<RGBSpectrum, Spectrum> *
    CreateImageSpectrumTexture(const Transform &tex2world, const TextureParams &tp)
    {
        std::unique_ptr<TextureMapping2D> map;
        std::string type = tp.FindString("mapping", "uv");
        if (type == "uv")
        {
            float su = tp.FindFloat("uscale", 1.);
            float sv = tp.FindFloat("vscale", 1.);
            float du = tp.FindFloat("udelta", 0.);
            float dv = tp.FindFloat("vdelta", 0.);
            map = std::make_unique<UVMapping2D>(su, sv, du, dv);
        }
        else if (type == "spherical")
            map = std::make_unique<SphericalMapping2D>(Inverse(tex2world));
        else if (type == "cylindrical")
            map = std::make_unique<CylindricalMapping2D>(Inverse(tex2world));
        else
        {
            Error("2D texture mapping \"%s\" unknown", type.c_str());
            map = std::make_unique<UVMapping2D>();
        }

        float maxAniso = tp.FindFloat("maxanisotropy", 8.f);
        bool trilerp = tp.FindBool("trilinear", false);
        std::string wrap = tp.FindString("wrap", "repeat");
        ImageWrap wrapMode = ImageWrap::Repeat;
        if (wrap == "black")
            wrapMode = ImageWrap::Black;
        else if (wrap == "clamp")
            wrapMode = ImageWrap::Clamp;
        float scale = tp.FindFloat("scale", 1.f);
        std::string filename = tp.FindFilename("filename");
        bool gamma = tp.FindBool("gamma", HasExtension(filename, ".tga") ||
                                          HasExtension(filename, ".png"));
        return new ImageTexture<RGBSpectrum, Spectrum>(
                std::move(map), filename, trilerp, maxAniso, wrapMode, scale, gamma
                );
    }

}

