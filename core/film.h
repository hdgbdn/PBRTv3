#ifndef PBRT_CORE_FILM_H
#define PBRT_CORE_FILM_H

#include "pbrt.h"
#include "geometry.h"
#include "filter.h"
#include "parallel.h"
#include "spectrum.h"

namespace pbrt
{
	class Film
	{
	public:
		Film(const Point2i& resolution, const Bounds2f& cropWindow,
			std::unique_ptr<Filter> filt, float diagonal,
			const std::string& filename, float scale);
		Bounds2i GetSampleBounds() const;
		Bounds2f GetPhysicalExtent() const;
		std::unique_ptr<FilmTile> GetFilmTile(const Bounds2i& sampleBounds);
		void MergeFilmTile(std::unique_ptr<FilmTile> tile);
		void SetImage(const Spectrum* img) const;
		void AddSplat(const Point2f& p, const Spectrum& v);
		void WriteImage(float splatScale = 1);
		const Point2i fullResolution;
		const float diagonal;
		std::unique_ptr<Filter> filter;
		const std::string filename;
		Bounds2i croppedPixelBounds;
	private:
		std::mutex mutex;
		const float scale;
		struct Pixel
		{
			float xyz[3] = { 0, 0, 0 };
			float filterWeightSum = 0;
			AtomicFloat splatXYZ[3];
			float pad;
		};
		std::unique_ptr<Pixel[]> pixels;
		static constexpr int filterTableWidth = 16;
		float filterTable[filterTableWidth * filterTableWidth];
		Pixel& GetPixel(const Point2i& p)
		{
			int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
			int offset = (p.x - croppedPixelBounds.pMin.x) + (p.y - croppedPixelBounds.pMin.y) * width;
			return pixels[offset];
		}
	};

	struct FilmTilePixel {
		Spectrum contribSum = 0.f;
		float filterWeightSum = 0.f;
	};

	class FilmTile
	{
	public:
		FilmTile(const Bounds2i& pixelBounds, const Vector2f& filterRadius,
			const float* filterTable, int filterTableSize);
		void AddSample(const Point2f& pFilm, Spectrum L,
			float sampleWeight = 1.);
		FilmTilePixel& GetPixel(const Point2i& p);
		Bounds2i GetPixelBounds() const { return pixelBounds; }
	private:
		const Bounds2i pixelBounds;
		const Vector2f filterRadius, invFilterRadius;
		const float* filterTable;
		const int filterTableSize;
		std::vector<FilmTilePixel> pixels;
	};
}

#endif