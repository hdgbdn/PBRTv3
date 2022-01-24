#include "imageio.h"
#include "spectrum.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stbimage.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbimage_write.h"

namespace pbrt
{
	std::unique_ptr<RGBSpectrum[]> ReadImage(const std::string& name,
		Point2i* resolution)
	{
        int n;
        unsigned char *data = stbi_load(name.c_str(), &resolution->x, &resolution->y, &n, 3);
		if(!data)
            Error("Unable to load image stored in format \"%s\" for filename \"%s\".",
                        strrchr(name.c_str(), '.') ? (strrchr(name.c_str(), '.') + 1)
                                                   : "(unknown)",
                        name.c_str());
        std::unique_ptr<RGBSpectrum[]> ret(new RGBSpectrum[resolution->x * resolution->y]);
        for (int y = 0; y < resolution->y; y++)
            for (int x = 0; x < resolution->x; x++)
            {
                int index = x + y * resolution->x;
                float src[3];
                src[0] = data[index] / 255.f;;
                src[1] = data[index + 1] / 255.f;;
                src[2] = data[index + 2] / 255.f;;

                ret[index] = RGBSpectrum::FromRGB(src);
            }

        stbi_image_free(data);
        return std::move(ret);
	}

	void WriteImage(const std::string& name, const float* rgb, const Bounds2i& outputBounds,
		const Point2i& totalResolution)
	{
        if (!stbi_write_hdr(
            name.c_str(), totalResolution.x, totalResolution.y, 3, rgb))
        {
            Error("Write image error");
        }
        return;
	}
}
