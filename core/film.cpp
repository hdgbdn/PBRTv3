#include "film.h"
#include "paramset.h"

namespace pbrt
{
	Film::Film(const Point2i& resolution, const Bounds2f& cropWindow, std::unique_ptr<Filter> filt, float diagonal,
	           const std::string& filename, float scale, float maxSampleLuminance)
		: fullResolution(resolution), diagonal(diagonal), filter(std::move(filter)),
		  filename(filename),
		  croppedPixelBounds(Point2i(std::ceil(fullResolution.x * cropWindow.pMin.x),
		                             std::ceil(fullResolution.y * cropWindow.pMin.y)),
		                     Point2i(std::ceil(fullResolution.x * cropWindow.pMax.x),
		                             std::ceil(fullResolution.y * cropWindow.pMax.y))),
		  scale(scale), pixels(std::unique_ptr<Pixel[]>(new Pixel[croppedPixelBounds.Area()]))
	{
		int offset = 0;
		for(int y = 0; y < filterTableWidth; ++y)
			for(int x = 0; x <filterTableWidth; ++x)
			{
				Point2f p;
				p.x = (x + .5f) * filter->radius.x / filterTableWidth;
				p.y = (y + .5f) * filter->radius.y / filterTableWidth;
				filterTable[++offset] = filter->Evaluate(p);
			}
	}

	Bounds2i Film::GetSampleBounds() const
	{
		Bounds2f floatBounds
		(
			Floor(Point2f(croppedPixelBounds.pMin) + Vector2f(.5f, .5f) - filter->radius),
			Ceil(Point2f(croppedPixelBounds.pMax) - Vector2f(.5f, .5f) + filter->radius)
		);
		return static_cast<Bounds2<int>>(floatBounds);
	}

	Bounds2f Film::GetPhysicalExtent() const
	{
		float aspect = (float)fullResolution.y / (float)fullResolution.x;
		float x = std::sqrt(diagonal * diagonal / (1 + aspect * aspect));
		float y = aspect * x;
		return Bounds2f(Point2f(-x / 2, -y / 2), Point2f(x / 2, y / 2));
	}

	std::unique_ptr<FilmTile> Film::GetFilmTile(const Bounds2i& sampleBounds)
	{
		Vector2f halfPixel = Vector2f(.5f, .5f);
		Bounds2f floatBounds = static_cast<Bounds2f>(sampleBounds);
		Point2i p0 = static_cast<Point2i>(Ceil(floatBounds.pMin - halfPixel - filter->radius));
		Point2i p1 = static_cast<Point2i>(Floor(floatBounds.pMax - halfPixel + filter->radius))
			+ Point2i(1, 1);
		Bounds2i tilePixelBounds =
			Intersect(Bounds2i(p0, p1), croppedPixelBounds);

		return std::unique_ptr<FilmTile>(new FilmTile(tilePixelBounds, filter->radius, filterTable, filterTableWidth));
	}

	void Film::MergeFilmTile(std::unique_ptr<FilmTile> tile)
	{
		std::lock_guard<std::mutex> lock(mutex);
		for(Point2i pixel:tile->GetPixelBounds())
		{
			const FilmTilePixel& tilePixel = tile->GetPixel(pixel);
			Pixel& mergePixel = GetPixel(pixel);
			float xyz[3];
			tilePixel.contribSum.ToXYZ(xyz);
			for (int i = 0; i < 3; ++i)
				mergePixel.xyz[i] += xyz[i];
			mergePixel.filterWeightSum += tilePixel.filterWeightSum;
		}
	}

	void Film::SetImage(const Spectrum* img) const
	{
		const int nPixels = croppedPixelBounds.Area();
		for(int i = 0; i < nPixels; ++i)
		{
			Pixel& p = pixels[i];
			img[i].ToXYZ(p.xyz);
			p.filterWeightSum = 1;
			p.splatXYZ[0] = p.splatXYZ[1] = p.splatXYZ[2] = 0;
		}
	}

	void Film::AddSplat(const Point2f& p, const Spectrum& v)
	{
		if (!InsideExclusive(static_cast<Point2i>(p), croppedPixelBounds))
			return;
		float xyz[3];
		v.ToXYZ(xyz);
		Pixel& pixel = GetPixel(static_cast<Point2i>(p));
		for (int i = 0; i < 3; ++i)
			pixel.splatXYZ[i].Add(xyz[i]);
	}

	void Film::WriteImage(float splatScale)
	{
		std::unique_ptr<float[]> rgb(new float[3 * croppedPixelBounds.Area()]);
		int offset = 0;
		for(Point2i p : croppedPixelBounds)
		{
			Pixel& pixel = GetPixel(p);
			XYZToRGB(pixel.xyz, &rgb[3 * offset]);
			const float filterWeightSum = pixel.filterWeightSum;
			if (filterWeightSum != 0)
			{
				float invWt = static_cast<float>(1) / filterWeightSum;
				rgb[3 * offset] = std::max(static_cast<float>(0), rgb[3 * offset] * invWt);
				rgb[3 * offset + 1] = std::max(static_cast<float>(0), rgb[3 * offset + 1] * invWt);
				rgb[3 * offset + 2] = std::max(static_cast<float>(0), rgb[3 * offset + 2] * invWt);
			}
			float splatRGB[3];
			float splatXYZ[3] = { pixel.splatXYZ[0], pixel.splatXYZ[1],
								  pixel.splatXYZ[2] };
			XYZToRGB(splatXYZ, splatRGB);
			rgb[3 * offset] += splatScale * splatRGB[0];
			rgb[3 * offset + 1] += splatScale * splatRGB[1];
			rgb[3 * offset + 2] += splatScale * splatRGB[2];

			rgb[3 * offset] *= scale;
			rgb[3 * offset + 1] *= scale;
			rgb[3 * offset + 2] *= scale;
			++offset;
		}
	}

	FilmTile::FilmTile(const Bounds2i& pixelBounds, const Vector2f& filterRadius, const float* filterTable,
	                   int filterTableSize)
		: pixelBounds(pixelBounds), filterRadius(filterRadius),
		  invFilterRadius(1 / filterRadius.x, 1 / filterRadius.y),
		  filterTable(filterTable), filterTableSize(filterTableSize),
		  pixels(std::max(0, pixelBounds.Area()))
	{
	}

	void FilmTile::AddSample(const Point2f& pFilm, Spectrum L, float sampleWeight)
	{
		Point2f pFilmDiscrete = pFilm - Vector2f(.5f, .5f);
		Point2i p0 = static_cast<Point2i>(Ceil(pFilmDiscrete - filterRadius));
		Point2i p1 = static_cast<Point2i>(Floor(pFilmDiscrete + filterRadius)) + Point2i(1, 1);
		int* ifx = ALLOCA(int, p1.x - p0.x);
		for (int x = p0.x; x < p1.x; ++x)
		{
			float fx = std::abs((x - pFilmDiscrete.x) * invFilterRadius.x * filterTableSize);
			ifx[x - p0.x] = std::min(static_cast<int>(std::floor(fx)), filterTableSize - 1);
		}
		int* ify = ALLOCA(int, p1.y - p0.y);
		for (int y = p0.x; y < p1.x; ++y)
		{
			float fy = std::abs((y - pFilmDiscrete.y) * invFilterRadius.y * filterTableSize);
			ify[y - p0.y] = std::min(static_cast<int>(std::floor(fy)), filterTableSize - 1);
		}
		for(int y = p0.y; y < p1.y; ++y)
			for(int x = p0.x; x < p1.x; ++x)
			{
				int offset = ify[y - p0.y] * filterTableSize + ifx[x - p0.x];
				float filterWeight = filterTable[offset];
				FilmTilePixel& pixel = GetPixel(Point2i(x, y));
				pixel.contribSum += L * sampleWeight * filterWeight;
				pixel.filterWeightSum += filterWeight;
			}
	}

	FilmTilePixel& FilmTile::GetPixel(const Point2i& p)
	{
		int width = pixelBounds.pMax.x - pixelBounds.pMin.x;
		int offset = (p.x - pixelBounds.pMin.x) + (p.y - pixelBounds.pMin.y) * width;
		return pixels[offset];
	}

	Film* CreateFilm(const ParamSet& params, std::unique_ptr<Filter> filter)
	{
		std::string filename;
		if (!PbrtOptions.imageFile.empty())
		{
			filename = PbrtOptions.imageFile;
			std::string paramsFilename = params.FindOneString("filename", "");
			if (!paramsFilename.empty())
				Warning(
					"Output filename supplied on command line, \"%s\" is overriding "
					"filename provided in scene description file, \"%s\".",
					PbrtOptions.imageFile.c_str(), paramsFilename.c_str());
		}
		else
			filename = params.FindOneString("filename", "pbrt.exr");

		int xres = params.FindOneInt("xresolution", 1280);
		int yres = params.FindOneInt("yresolution", 720);

		Bounds2f crop;
		int cwi;
		const float* cr = params.FindFloat("cropwindow", &cwi);
		if (cr && cwi == 4) {
			crop.pMin.x = Clamp(std::min(cr[0], cr[1]), 0.f, 1.f);
			crop.pMax.x = Clamp(std::max(cr[0], cr[1]), 0.f, 1.f);
			crop.pMin.y = Clamp(std::min(cr[2], cr[3]), 0.f, 1.f);
			crop.pMax.y = Clamp(std::max(cr[2], cr[3]), 0.f, 1.f);
		}
		else if (cr)
			Error("%d values supplied for \"cropwindow\". Expected 4.", cwi);
		else
			crop = Bounds2f(Point2f(Clamp(PbrtOptions.cropWindow[0][0], 0, 1),
				Clamp(PbrtOptions.cropWindow[1][0], 0, 1)),
				Point2f(Clamp(PbrtOptions.cropWindow[0][1], 0, 1),
					Clamp(PbrtOptions.cropWindow[1][1], 0, 1)));

		float scale = params.FindOneFloat("scale", 1.);
		float diagonal = params.FindOneFloat("diagonal", 35.);
		float maxSampleLuminance = params.FindOneFloat("maxsampleluminance",
			pbrt::Infinity);
		return new Film(Point2i(xres, yres), crop, std::move(filter), diagonal,
			filename, scale, maxSampleLuminance);
	}
}
