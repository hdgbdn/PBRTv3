#ifndef PBRT_CORE_STATS_H
#define PBRT_CORE_STATS_H

#include <functional>
#include <map>
#include "pbrt.h"

namespace pbrt
{
    class StatsAccumulator;

    class StatRegisterer
    {
    public:
        StatRegisterer(std::function<void(StatsAccumulator&)> func)
        {
            if (!funcs)
                funcs = new std::vector<std::function<void(StatsAccumulator&)>>;
            funcs->push_back(func);
        }
        static void CallCallbacks(StatsAccumulator& accum);
    private:
        static std::vector<std::function<void(StatsAccumulator&)>>* funcs;
    };

    class StatsAccumulator
    {
    public:
        void ReportCounter(const std::string& name, int64_t val)
        {
            counters[name] += val;
        }
    private:
        std::map<std::string, int64_t> counters;
    };
#define STAT_COUNTER(title, var)                        \
static thread_local int64_t var;                        \
static void STATS_FUNC##var(StatsAccumulator &accum) {  \
    accum.ReportCounter(title, var);                    \
    var = 0;                                            \
}                                                       \
static StatRegisterer STATS_REG##var(STATS_FUNC##var)

    void ReportThreadStats();

    enum class Prof
    {
        IntegratorRender,
        SamplerIntegratorLi,
        DirectLighting,
        AccelIntersect,
        AccelIntersectP,
        TriIntersect,
        TriIntersectP,
        ComputeScatteringFuncs,
        GenerateCameraRay,
        BSDFEvaluation,
        BSSRDFEvaluation,
        MergeFilmTile,
        SplatFilm,
        StartPixel,
        TexFiltTrilerp,
        TexFiltEWA,
        NumProfEvents
    };

    extern thread_local uint32_t ProfilerState;

    inline uint32_t CurrentProfilerState() { return ProfilerState; }
}

#endif