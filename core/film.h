#ifndef PBRT_CORE_FILM_H
#define PBRT_CORE_FILM_H

#include "pbrt.h"
#include "geometry.h"
#include "spectrum.h"

namespace pbrt
{
	class Film
	{
	public:
		Bounds2i GetSamplBounds();
		std::unique_ptr<FilmTile> GetFilmTile(const Bounds2i& sampleBounds);
		void MergeFilmTile(std::unique_ptr<FilmTile> tile);
		void WriteImage(float splatScale = 1);
		const Point2i fullResolution;
	};

	class FilmTile
	{
	public:
		void AddSample(const Point2f& pFilm, Spectrum L,
			float sampleWeight = 1.);
	};
}

#endif