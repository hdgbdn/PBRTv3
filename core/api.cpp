#include "api.h"
#include "error.h"
#include "transformation.h"
#include "spectrum.h"

#include <map>

#include "paramset.h"

namespace pbrt
{
	// API Global Variables
	Options PbrtOptions;

    // API Local Classes
    constexpr int MaxTransforms = 2;
    constexpr int StartTransformBits = 1 << 0;
    constexpr int EndTransformBits = 1 << 1;
    constexpr int ALLTransformsBits = (1 << MaxTransforms) - 1;

    struct TransformSet
    {
        Transform& operator[](int i)
        {
            return t[i];
        }

        bool IsAnimated() const {
            for (int i = 0; i < MaxTransforms - 1; ++i)
                if (t[i] != t[i + 1]) return true;
            return false;
        }

        friend TransformSet Inverse(const TransformSet& ts);
    private:
        Transform t[MaxTransforms];
    };

    struct RenderOptions
    {
        float transformStartTime = 0, transformEndTime = 1;
        std::string FilterName = "box";
        ParamSet FilterParams;
        std::string FilmName = "image";
        ParamSet FilmParams;
        std::string SamplerName = "halton";
        ParamSet SamplerParams;
        std::string AcceleratorName = "bvh";
        ParamSet AcceleratorParams;
        std::string IntegratorName = "path";
        ParamSet IntegratorParams;
        std::string CameraName = "perspective";
        ParamSet CameraParams;
    };

	// API Static Data
	enum class APIState { Uninitialized, OptionsBlock, WorldBlock };
	static APIState currentApiState = APIState::Uninitialized;
    static TransformSet curTransform;
    static int activeTransformBits = ALLTransformsBits;
    static std::map<std::string, TransformSet> namedCoordinateSystems;
    static std::unique_ptr<RenderOptions> renderOptions;

	// API Macros
#define VERIFY_INITIALIZED(func)                           \
    if (currentApiState == APIState::Uninitialized) {        \
        Error(                                             \
            "pbrtInit() must be before calling \"%s()\". " \
            "Ignoring.",                                   \
            func);                                         \
        return;                                            \
    } else /* else is used to swallow trailing semicolon */

#define VERIFY_OPTIONS(func)                             \
    VERIFY_INITIALIZED(func);                            \
    if (currentApiState == APIState::WorldBlock) {       \
        Error(                                           \
            "Options cannot be set inside world block; " \
            "\"%s\" not allowed.  Ignoring.",            \
            func);                                       \
        return;                                          \
    } else /* else is used to swallow trailing semicolon */

#define VERIFY_WORLD(func)                                   \
    VERIFY_INITIALIZED(func);                                \
    if (currentApiState == APIState::OptionsBlock) {         \
        Error(                                               \
            "Scene description must be inside world block; " \
            "\"%s\" not allowed. Ignoring.",                 \
            func);                                           \
        return;                                              \
    } else /* else is used to swallow trailing semicolon */

#define WARN_IF_ANIMATED_TRANSFORM(func)                              \
do { if (curTransform.IsAnimated())                                   \
         Warning("Animated transformations set; ignoring for \"%s\" " \
                 "and using the start transform only", func);         \
} while (false) /* else is used to swallow trailing semicolon */

#define FOR_ACTIVE_TRANSFORMS(expr)                   \
    for (int i = 0; i < MaxTransforms; ++i)           \
        if (activeTransformBits & (1 << i)) { expr }

    void pbrtIdentity()
    {
        VERIFY_INITIALIZED("Identiy");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = Transform();)
    }

    void pbrtTranslate(float dx, float dy, float dz)
    {
        VERIFY_INITIALIZED("Translate");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * Translate(Vector3f(dx, dy, dz));)
    }

    void pbrtRotate(float angle, float ax, float ay, float az)
    {
        VERIFY_INITIALIZED("Rotate");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * Rotate(angle, Vector3f(ax, ay, az));)
    }

    void pbrtScale(float sx, float sy, float sz)
    {
        VERIFY_INITIALIZED("Scale");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * Scale(sx, sy, sz);)
    }

    void pbrtLookAt(float ex, float ey, float ez,
        float lx, float ly, float lz,
        float ux, float uy, float uz)
    {
        VERIFY_INITIALIZED("LookAt");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * LookAt(Point3f(ex, ey, ez), Point3f(lx, ly, lz), Vector3f(ux, uy, uz));)
    }

    void pbrtConcatTransform(float transform[16])
    {
        VERIFY_INITIALIZED("ConcatTransform");
        FOR_ACTIVE_TRANSFORMS(
            curTransform[i] = curTransform[i] * Transform(Matrix4x4(
                transform[0], transform[4], transform[8], transform[12],
                transform[1], transform[5], transform[9], transform[13],
                transform[2], transform[6], transform[10], transform[14],
                transform[3], transform[7], transform[11], transform[15]));
        )
    }

    void pbrtTransform(float transform[16])
    {
        VERIFY_INITIALIZED("ConcatTransform");
        FOR_ACTIVE_TRANSFORMS(
            curTransform[i] = Transform(Matrix4x4(
                transform[0], transform[4], transform[8], transform[12],
                transform[1], transform[5], transform[9], transform[13],
                transform[2], transform[6], transform[10], transform[14],
                transform[3], transform[7], transform[11], transform[15]));
        )
    }

    void pbrtCoordinateSystem(const std::string& name)
    {
        VERIFY_INITIALIZED("CoordinateSystem");
        namedCoordinateSystems[name] = curTransform;
    }

    void pbrtCoordSysTransform(const std::string& name)
    {
        VERIFY_INITIALIZED("CoordSysTransform");
        if (namedCoordinateSystems.find(name) != namedCoordinateSystems.end())
            curTransform = namedCoordinateSystems[name];
        else
            Warning("Couldn't find named coordinate system \"%s\"",
                name.c_str());
    }

    void pbrtActiveTransformAll()
    {
        activeTransformBits = ALLTransformsBits;
    }

    void pbrtActiveTransformEndTime()
    {
        activeTransformBits = EndTransformBits;
    }

    void pbrtActiveTransformStartTime()
    {
        activeTransformBits = StartTransformBits;
    }

    void pbrtTransformTimes(float start, float end)
    {
        VERIFY_OPTIONS("TransformTimes");
        renderOptions->transformStartTime = start;
        renderOptions->transformEndTime = end;
    }

    void pbrtPixelFilter(const std::string& name, const ParamSet& params)
    {
        VERIFY_OPTIONS("PiexlFilter");
        renderOptions->FilterName = name;
        renderOptions->FilterParams = params;
    }

    void pbrtFilm(const std::string& type, const ParamSet& params)
    {
        VERIFY_OPTIONS("PiexlFilter");
        renderOptions->FilmName = type;
        renderOptions->FilmParams = params;
    }

    void pbrtSampler(const std::string& name, const ParamSet& params)
    {
        VERIFY_OPTIONS("Sampler");
        renderOptions->SamplerName = name;
        renderOptions->SamplerParams = params;
    }

    void pbrtAccelerator(const std::string& name, const ParamSet& params)
    {
        VERIFY_OPTIONS("Accelerator");
        renderOptions->AcceleratorName = name;
        renderOptions->AcceleratorParams = params;
    }

    void pbrtIntegrator(const std::string& name, const ParamSet& params)
    {
        VERIFY_OPTIONS("Accelerator");
        renderOptions->IntegratorName = name;
        renderOptions->IntegratorParams = params;
    }

	void pbrtInit(const Options& opt)
	{
		PbrtOptions = opt;
		if (currentApiState != APIState::Uninitialized) 
			Error("pbrtInit() has already been called.");
		currentApiState = APIState::OptionsBlock;

        renderOptions.reset(new RenderOptions);

		SampledSpectrum::Init();
	}

	void pbrtCleanUp()
	{
		if (currentApiState == APIState::Uninitialized)
			Error("pbrtCleanup() called without pbrtInit()");
		else if (currentApiState == APIState::WorldBlock)
			Error("pbrtCleanup() called while inside world block");
        renderOptions.reset(nullptr);
	}

    TransformSet Inverse(const TransformSet& ts)
    {
        TransformSet tInv;
        for (int i = 0; i < MaxTransforms; ++i)
            tInv.t[i] = Inverse(ts.t[i]);
        return tInv;
    }
}
