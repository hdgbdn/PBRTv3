#include "api.h"
#include "error.h"
#include "transformation.h"
#include "spectrum.h"
#include <map>
#include <utility>
#include "medium.h"
#include "memory.h"
#include "paramset.h"
#include "primitive.h"
#include "bvh.h"
#include "integrator.h"
#include "scene.h"
#include "stats.h"

// material header
#include "matte.h"
#include "plastic.h"

// shape header
#include "triangle.h"
#include "sphere.h"
#include "disk.h"

// light header
#include "infinite.h"
#include "point.h"
#include "spot.h"

// image header
#include "box.h"
#include "diffuse.h"
#include "imagemap.h"
#include "path.h"
#include "perspective.h"
#include "scale.h"
#include "stratified.h"

namespace pbrt
{
	// API Global Variables
	Options PbrtOptions;

    // API Local Classes
    constexpr int MaxTransforms = 2;
    constexpr int StartTransformBits = 1 << 0;
    constexpr int EndTransformBits = 1 << 1;
    constexpr int ALLTransformsBits = (1 << MaxTransforms) - 1;

    // forward declaration
    std::shared_ptr<Material> MakeMaterial(const std::string &name, const TextureParams &mp);

    struct TransformSet
    {
        Transform& operator[](int i)
        {
            return t[i];
        }

        const Transform& operator[](int i) const
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
        TransformSet CameraToWorld;
        std::map<std::string, std::shared_ptr<Medium>> namedMedia;
        std::vector<std::shared_ptr<Light>> lights;

        std::map<std::string, std::vector<std::shared_ptr<Primitive>>> instances;
        std::vector<std::shared_ptr<Primitive>>* currentInstance = nullptr;
        std::vector<std::shared_ptr<Primitive>> primitives;

        Camera* MakeCamera() const;
        Scene* MakeScene();
        Integrator* MakeIntegrator() const;
    };

    struct MaterialInstance {
        MaterialInstance() = default;
        MaterialInstance(std::string name, std::shared_ptr<Material> mtl,
                         ParamSet params)
                : name(std::move(name)), material(std::move(mtl)), params(std::move(params)) {}

        std::string name;
        std::shared_ptr<Material> material;
        ParamSet params;
    };

    struct GraphicsState
    {
        GraphicsState()
        : floatTextures(std::make_shared<FloatTextureMap>()),
          spectrumTextures(std::make_shared<SpectrumTextureMap>()),
          namedMaterials(std::make_shared<NamedMaterialMap>())
        {
            ParamSet empty;
            TextureParams tp(empty, empty, *floatTextures, *spectrumTextures);
            std::shared_ptr<Material> mtl(CreateMatteMaterial(tp));
            currentMaterial = std::make_shared<MaterialInstance>("matte", mtl, ParamSet());
        }
        std::string currentInsideMedium, currentOutsideMedium;
        using FloatTextureMap = std::map<std::string, std::shared_ptr<Texture<float>>>;
        std::shared_ptr<FloatTextureMap> floatTextures;
        bool floatTexturesShared = false;

        using SpectrumTextureMap = std::map<std::string, std::shared_ptr<Texture<Spectrum>>>;
        std::shared_ptr<SpectrumTextureMap> spectrumTextures;
        bool spectrumTexturesShared = false;

        using NamedMaterialMap = std::map<std::string, std::shared_ptr<MaterialInstance>>;
        std::shared_ptr<NamedMaterialMap> namedMaterials;
        bool namedMaterialsShared = false;

        std::shared_ptr<MaterialInstance> currentMaterial;
        ParamSet areaLightParams;
        std::string areaLight;
        bool reverseOrientation = false;

        MediumInterface CreateMediumInterface();
        std::shared_ptr<Material> GetMaterialForShape(const ParamSet &shapeParams);
    };

    MediumInterface GraphicsState::CreateMediumInterface()
    {
        return MediumInterface();
    }

    // Attempt to determine if the ParamSet for a shape may provide a value for
    // its material's parameters. Unfortunately, materials don't provide an
    // explicit representation of their parameters that we can query and
    // cross-reference with the parameter values available from the shape.
    //
    // Therefore, we'll apply some "heuristics".
    bool shapeMaySetMaterialParameters(const ParamSet &ps) {
        for (const auto &param : ps.textures)
            // Any texture other than one for an alpha mask is almost certainly
            // for a Material (or is unused!).
            if (param->name != "alpha" && param->name != "shadowalpha")
                return true;

        // Special case spheres, which are the most common non-mesh primitive.
        for (const auto &param : ps.floats)
            if (param->nValues == 1 && param->name != "radius")
                return true;

        // Extra special case strings, since plymesh uses "filename", curve "type",
        // and loopsubdiv "scheme".
        for (const auto &param : ps.strings)
            if (param->nValues == 1 && param->name != "filename" &&
                param->name != "type" && param->name != "scheme")
                return true;

        // For all other parameter types, if there is a single value of the
        // parameter, assume it may be for the material. This should be valid
        // (if conservative), since no materials currently take array
        // parameters.
        for (const auto &param : ps.bools)
            if (param->nValues == 1)
                return true;
        for (const auto &param : ps.ints)
            if (param->nValues == 1)
                return true;
        for (const auto &param : ps.point2fs)
            if (param->nValues == 1)
                return true;
        for (const auto &param : ps.vector2fs)
            if (param->nValues == 1)
                return true;
        for (const auto &param : ps.point3fs)
            if (param->nValues == 1)
                return true;
        for (const auto &param : ps.vector3fs)
            if (param->nValues == 1)
                return true;
        for (const auto &param : ps.normal3fs)
            if (param->nValues == 1)
                return true;
        for (const auto &param : ps.spectra)
            if (param->nValues == 1)
                return true;

        return false;
    }

    std::shared_ptr<Material> GraphicsState::GetMaterialForShape(const ParamSet &shapeParams)
    {
        // CHECK(currentMaterial);
        if (shapeMaySetMaterialParameters(shapeParams)) {
            // Only create a unique material for the shape if the shape's
            // parameters are (apparently) going to provide values for
            // some of the material parameters.
            TextureParams mp(shapeParams, currentMaterial->params, *floatTextures,
                             *spectrumTextures);
            return MakeMaterial(currentMaterial->name, mp);
        } else
            return currentMaterial->material;
    }

    class TransformCache
    {
    public:
        TransformCache() = default;
		void Lookup(const Transform& trans, Transform** origin, Transform** inverse)
		{
			if (cache.find(trans) != cache.end())
			{
                if (origin) *origin = cache[trans].first;
                if (inverse) *inverse = cache[trans].second;
			}
            else
            {
                Transform* t = arena.Alloc<Transform>();
                Transform* tr = arena.Alloc<Transform>();
                *t = trans;
                *tr = Inverse(trans);
                cache[trans] = std::make_pair(t, tr);
                if (origin) *origin = t;
                if (inverse) *inverse = tr;
            }
		}
    private:
        std::map<Transform, std::pair<Transform*, Transform*>> cache;
        MemoryArena arena;
    };

	// API Static Data
	enum class APIState { Uninitialized, OptionsBlock, WorldBlock };
	static APIState currentApiState = APIState::Uninitialized;
    static TransformSet curTransform;
    static int activeTransformBits = ALLTransformsBits;
    static std::map<std::string, TransformSet> namedCoordinateSystems;
    static std::unique_ptr<RenderOptions> renderOptions;
    static GraphicsState graphicsState;
    static std::vector<GraphicsState> pushedGraphicsStates;
    static std::vector<TransformSet> pushedTransforms;
    static std::vector<uint32_t> pushedActiveTransformBits;
    static TransformCache transformCache;

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

    // Object Creation Function Definitions
	Camera* MakeCamera(const std::string& name, const ParamSet& paramSet, 
        const TransformSet& cam2worldSet, float transformStart, float transformEnd, Film* film)
	{
        Camera* camera = nullptr;
        MediumInterface mediumInterface = graphicsState.CreateMediumInterface();
        Transform* cam2World[2];
        transformCache.Lookup(cam2worldSet[0], &cam2World[0], nullptr);
        transformCache.Lookup(cam2worldSet[1], &cam2World[1], nullptr);
        AnimatedTransform animatedCam2World(cam2World[0], transformStart,
            cam2World[1], transformEnd);
        if (name == "perspective")
            camera = CreatePerspectiveCamera(paramSet, animatedCam2World, film,
                mediumInterface.outside);
        return camera;
	}

    std::unique_ptr<Filter> MakeFilter(const std::string& name, const ParamSet& paramSet)
	{
        Filter* filter = nullptr;
        if (name == "box")
            filter = CreateBoxFilter(paramSet);
		else 
        {
            Error("Filter \"%s\" unknown.", name.c_str());
            exit(1);
        }
        paramSet.ReportUnused();
        return std::unique_ptr<Filter>(filter);
	}

    Film* MakeFilm(const std::string& name, const ParamSet& paramSet, std::unique_ptr<Filter> filter)
	{
        Film* film = nullptr;
        if (name == "image")
            film = CreateFilm(paramSet, std::move(filter));
        else
            Warning("Film \"%s\" unknown.", name.c_str());
        paramSet.ReportUnused();
        return film;
	}

    std::shared_ptr<Sampler> MakeSampler(const std::string& name,
                                         const ParamSet& paramSet,
                                         const Film* film)
	{
        Sampler* sampler = nullptr;
        if (name == "stratified")
            sampler = CreateStratifiedSampler(paramSet);
        else
            Warning("Sampler \"%s\" unknown.", name.c_str());
        paramSet.ReportUnused();
        return std::shared_ptr<Sampler>(sampler);
	}

    STAT_COUNTER("Scene/Materials created", nMaterialsCreated);

    std::shared_ptr<Material> MakeMaterial(const std::string& name,
                                           const TextureParams& mp)
    {
        Material *material = nullptr;
        if (name.empty() || name == "none")
            return nullptr;
        else if (name == "matte")
            material = CreateMatteMaterial(mp);
        else if (name == "plastic")
            material = CreatePlasticMaterial(mp);
        else {
            Warning("Material \"%s\" unknown. Using \"matte\".", name.c_str());
            material = CreateMatteMaterial(mp);
        }

        mp.ReportUnused();
        if (!material) Error("Unable to create material \"%s\"", name.c_str());
        else ++nMaterialsCreated;
        return std::shared_ptr<Material>(material);
    }

    std::shared_ptr<Texture<float>> MakeFloatTexture(const std::string& name,
        const Transform& tex2world,
        const TextureParams& tp)
    {
        Texture<float>* tex = nullptr;
        if (name == "imagemap")
            tex = CreateImageFloatTexture(tex2world, tp);
        else if(name == "scale")
            tex = CreateScaleFloatTexture(tex2world, tp);
        else
            Warning("Spectrum texture \"%s\" unknown.", name.c_str());
        tp.ReportUnused();
        return std::shared_ptr<Texture<float>>(tex);
    }

    std::shared_ptr<Texture<Spectrum>> MakeSpectrumTexture(const std::string& name,
        const Transform& tex2world,
        const TextureParams& tp)
    {
        Texture<Spectrum>* tex = nullptr;
        if (name == "imagemap")
            tex = CreateImageSpectrumTexture(tex2world, tp);
        else
            Warning("Spectrum texture \"%s\" unknown.", name.c_str());
        tp.ReportUnused();
        return std::shared_ptr<Texture<Spectrum>>(tex);
    }

    std::shared_ptr<Light> MakeLight(const std::string& name, const ParamSet& params, 
        const Transform& light2World, const MediumInterface& mediumInterface)
    {
        std::shared_ptr<Light> light;
        if (name == "infinite")
            light = CreateInfinitedLight(light2World, params);
        else if (name == "point")
            light = CreatePointLight(light2World, mediumInterface.outside, params);
        else if (name == "spot")
            light = CreateSpotLight(light2World, mediumInterface.outside, params);
        else
            Warning("Light \"%s\" unknown.", name.c_str());
        params.ReportUnused();
        return light;
    }

    std::shared_ptr<AreaLight> MakeAreaLight(const std::string& name, const Transform& light2World,
        const MediumInterface& mi, const ParamSet& params, std::shared_ptr<Shape> shape)
    {
        std::shared_ptr<AreaLight> area;
        if (name == "area" || name == "diffuse")
            area = CreateDiffuseAreaLight(light2World, mi.outside,
                params, shape);
        else
            Warning("Area light \"%s\" unknown.", name.c_str());
        params.ReportUnused();
        return area;
    }

    std::vector<std::shared_ptr<Shape>> MakeShapes(const std::string& name, const Transform* ObjectToWorld,
                                                   const Transform* WorldToObject, bool reverseOrientation,
                                                   const ParamSet& paramSet)
    {
	    std::vector<std::shared_ptr<Shape>> shapes;
        std::shared_ptr<Shape> s;
        if (name == "sphere")
            s = CreateSphereShape(ObjectToWorld, WorldToObject, reverseOrientation, paramSet);
        else if (name == "disk")
            s = CreateDiskShape(ObjectToWorld, WorldToObject, reverseOrientation, paramSet);
        if (s != nullptr) shapes.push_back(s);
        else if (name == "trianglemesh")
        {
            shapes = CreateTriangleMeshShape(ObjectToWorld, WorldToObject,
                                             reverseOrientation, paramSet,
                                             &*graphicsState.floatTextures);
        }
        return shapes;
    }

    std::shared_ptr<Primitive> MakeAccelerator(const std::string& name, std::vector<std::shared_ptr<Primitive>>, const ParamSet& paramSet)
    {
        return {};
    }

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

    void pbrtCamera(const std::string& name, const ParamSet& params)
    {
        VERIFY_OPTIONS("Camera");
        renderOptions->CameraName = name;
        renderOptions->CameraParams = params;
        renderOptions->CameraToWorld = Inverse(curTransform);
        namedCoordinateSystems["camera"] = renderOptions->CameraToWorld;
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

    void pbrtMediumInterface(const std::string& insideName, const std::string& outsideName)
    {
        VERIFY_INITIALIZED("MediumInterface");
        graphicsState.currentInsideMedium = insideName;
        graphicsState.currentOutsideMedium = outsideName;
    }

    void pbrtInit(const Options& opt)
	{
		PbrtOptions = opt;
		if (currentApiState != APIState::Uninitialized) 
			Error("pbrtInit() has already been called.");
		currentApiState = APIState::OptionsBlock;

        renderOptions.reset(new RenderOptions);
        graphicsState = GraphicsState();
        ParallelInit();
		SampledSpectrum::Init();
	}

    void pbrtWorldBegin()
    {
        VERIFY_OPTIONS("WorldBegin");
        currentApiState = APIState::WorldBlock;
        for (int i = 0; i < MaxTransforms; ++i)
            curTransform[i] = Transform();
        activeTransformBits = ALLTransformsBits;
        namedCoordinateSystems["world"] = curTransform;
    }

    void pbrtWorldEnd()
    {
        VERIFY_WORLD("WorldEnd");
        while(!pushedGraphicsStates.empty())
        {
	        Warning("Missing end to pbrtAttributeBegin()");
            pushedGraphicsStates.pop_back();
            pushedTransforms.pop_back();
        }
        while (pushedTransforms.size()) {
            Warning("Missing end to pbrtTransformBegin()");
            pushedTransforms.pop_back();
        }
        std::unique_ptr<Scene> scene(renderOptions->MakeScene());
        std::unique_ptr<Integrator> integrator(renderOptions->MakeIntegrator());
        if (scene && integrator) integrator->Render(*scene);

        graphicsState = GraphicsState();
        //transformCache.Clear();
        currentApiState = APIState::OptionsBlock;
        //ImageTexture<Float, Float>::ClearCache();
        //ImageTexture<RGBSpectrum, Spectrum>::ClearCache();
        renderOptions.reset(new RenderOptions);

        //TerminateWorkerThreads();
    }

    void pbrtAttributeBegin()
    {
        VERIFY_WORLD("AttributeBegin");
        pushedGraphicsStates.push_back(graphicsState);
        graphicsState.floatTexturesShared = graphicsState.spectrumTexturesShared =
            graphicsState.namedMaterialsShared = true;
        pushedTransforms.push_back(curTransform);
        pushedActiveTransformBits.push_back(activeTransformBits);
    }

    void pbrtAttributeEnd()
    {
        VERIFY_WORLD("AttrbuteEnd");
        if (pushedGraphicsStates.empty())
        {
            Error("Unmatched pbrtAttributeEnd() encountered. "
                "Ignoring it.");
            return;
        }
        graphicsState = std::move(pushedGraphicsStates.back());
        pushedGraphicsStates.pop_back();
        curTransform = pushedTransforms.back();
        pushedTransforms.pop_back();
        activeTransformBits = pushedActiveTransformBits.back();
        pushedActiveTransformBits.pop_back();
    }

    void pbrtTransformBegin()
    {
        VERIFY_WORLD("TransformBegin");
        pushedTransforms.push_back(curTransform);
        pushedActiveTransformBits.push_back(activeTransformBits);
    }

    void pbrtTransformEnd()
    {
        VERIFY_WORLD("TransformEnd");
        if (pushedTransforms.empty()) {
            Error(
                "Unmatched pbrtTransformEnd() encountered. "
                "Ignoring it.");
            return;
        }
        curTransform = pushedTransforms.back();
        pushedTransforms.pop_back();
        activeTransformBits = pushedActiveTransformBits.back();
        pushedActiveTransformBits.pop_back();
    }

    void pbrtObjectBegin(const std::string& name)
    {
        pbrtAttributeBegin();
        if (renderOptions->currentInstance) Error("ObjectBegin called inside of instance definition");
        renderOptions->instances[name] = std::vector<std::shared_ptr<Primitive>>();
        renderOptions->currentInstance = &renderOptions->instances[name];
    }

    void pbrtObjectEnd()
    {
        VERIFY_WORLD("ObjectEnd");
        if (!renderOptions->currentInstance) Error("ObjectEnd called outside of instance definition");
        renderOptions->currentInstance = nullptr;
        pbrtAttributeEnd();
    }

    void pbrtObjectInstance(const std::string& name)
    {
        VERIFY_WORLD("ObjectInstance");
        if (renderOptions->currentInstance)
        {
            Error("ObjectInstance can't be called inside instance definition");
            return;
        }
        if (!renderOptions->instances.contains(name)) 
        {
            Error("Unable to find instance named \"%s\"", name.c_str());
            return;
        }
        std::vector<std::shared_ptr<Primitive>>& in = renderOptions->instances[name];
        if (in.empty()) return;
        if (in.size() > 1)
        {
            std::shared_ptr<Primitive> accel(MakeAccelerator(renderOptions->AcceleratorName, in, renderOptions->AcceleratorParams));
            if (!accel) accel = std::make_shared<BVHAccel>(in);
            in.clear();
            in.push_back(accel);
        }
        Transform* InstanceToWorld[2];
        transformCache.Lookup(curTransform[0], &InstanceToWorld[0], nullptr);
        transformCache.Lookup(curTransform[1], &InstanceToWorld[1], nullptr);
        AnimatedTransform animatedInstanceToWorld(InstanceToWorld[0], renderOptions->transformStartTime, InstanceToWorld[1], renderOptions->transformEndTime);
    }

    void pbrtTexture(const std::string& name, const std::string& type, const std::string& texName, const ParamSet& params)
    {
        VERIFY_WORLD("Texture");
        TextureParams tp(params, params, *graphicsState.floatTextures, *graphicsState.spectrumTextures);
        if (type == "float")
        {
            if (graphicsState.floatTextures->find(name) != graphicsState.floatTextures->end())
                Info("Texture \"%s\" being redefined", name.c_str());
            WARN_IF_ANIMATED_TRANSFORM("Texture");
            std::shared_ptr<Texture<float>> ft = MakeFloatTexture(texName, curTransform[0], tp);
            if (ft) (*graphicsState.floatTextures)[name] = ft;
        }
        else if (type == "color" || type == "spectrum")
        {
            if (graphicsState.spectrumTextures->find(name) != graphicsState.spectrumTextures->end())
                Info("Texture \"%s\" being redefined", name.c_str());
            WARN_IF_ANIMATED_TRANSFORM("Texture");
            std::shared_ptr<Texture<Spectrum>> st = MakeSpectrumTexture(texName,
                curTransform[0], tp);
            if (st) (*graphicsState.spectrumTextures)[name] = st;
        }
        else
            Error("Texture type \"%s\" unknown.", type.c_str());
    }

    void pbrtMaterial(const std::string &name, const ParamSet &params) {
        VERIFY_WORLD("Material");
        ParamSet emptyParams;
        TextureParams mp(params, emptyParams, *graphicsState.floatTextures,
                         *graphicsState.spectrumTextures);
        std::shared_ptr<Material> mtl = MakeMaterial(name, mp);
        graphicsState.currentMaterial =
                std::make_shared<MaterialInstance>(name, mtl, params);
    }

    void pbrtLightSource(const std::string& name, const ParamSet& params)
    {
        VERIFY_WORLD("LightSource");
        WARN_IF_ANIMATED_TRANSFORM("LightSource");
        MediumInterface mi = graphicsState.CreateMediumInterface();
        std::shared_ptr<Light> lt = MakeLight(name, params, curTransform[0], mi);
        if (!lt) Error("LightSource: light type \"%s\" unknown.", name.c_str());
        else renderOptions->lights.push_back(lt);
    }

    void pbrtAreaLightSource(const std::string& name, const ParamSet& params)
    {
        VERIFY_WORLD("AREALIGHTSOURCE");
        graphicsState.areaLight = name;
        graphicsState.areaLightParams = params;
    }

    void pbrtShape(const std::string& name, const ParamSet& params)
    {
        VERIFY_WORLD("Shape");
        std::vector<std::shared_ptr<Primitive>> prims;
        std::vector<std::shared_ptr<AreaLight>> areaLights;
        if (!curTransform.IsAnimated())
        {
            Transform* ObjToWorld, * WorldToObj;
            transformCache.Lookup(curTransform[0], &ObjToWorld, &WorldToObj);
            std::shared_ptr<Material> mtl = graphicsState.GetMaterialForShape(params);
            std::vector<std::shared_ptr<Shape>> shapes =
                    MakeShapes(name, ObjToWorld, WorldToObj,
                               graphicsState.reverseOrientation, params);
            if (shapes.empty()) return;
            params.ReportUnused();
            MediumInterface mi = graphicsState.CreateMediumInterface();
            for (auto s : shapes)
            {
                std::shared_ptr<AreaLight> area;
                if (!graphicsState.areaLight.empty())
                {
                    area = MakeAreaLight(graphicsState.areaLight, curTransform[0], mi, 
                        graphicsState.areaLightParams, s);
                    areaLights.push_back(area);
                }
                prims.push_back(std::make_shared<GeometricPrimitive>(s, mtl, area, mi));
            }
        }
        else
        {
	        // for animated shape
        }
        if (renderOptions->currentInstance != nullptr)
        {
	        if (!areaLights.empty()) Warning("Area lights not supported with object instancing");
            renderOptions->currentInstance->insert(renderOptions->currentInstance->end(), prims.begin(), prims.end());
        }
        else
        {
            renderOptions->primitives.insert(renderOptions->primitives.end(),
                prims.begin(), prims.end());
            if (!areaLights.empty())
                renderOptions->lights.insert(renderOptions->lights.end(),
                    areaLights.begin(), areaLights.end());
        }
    }

    void pbrtCleanUp()
	{
		if (currentApiState == APIState::Uninitialized)
			Error("pbrtCleanup() called without pbrtInit()");
		else if (currentApiState == APIState::WorldBlock)
			Error("pbrtCleanup() called while inside world block");
        currentApiState = APIState::Uninitialized;
        ParallelCleanup();
        renderOptions.reset(nullptr);
	}

    TransformSet Inverse(const TransformSet& ts)
    {
        TransformSet tInv;
        for (int i = 0; i < MaxTransforms; ++i)
            tInv.t[i] = Inverse(ts.t[i]);
        return tInv;
    }

    Camera* RenderOptions::MakeCamera() const {
        std::unique_ptr<Filter> filter = MakeFilter(FilterName, FilterParams);
        Film* film = MakeFilm(FilmName, FilmParams, std::move(filter));
        if (!film) {
            Error("Unable to create film.");
            return nullptr;
        }
        Camera* camera = pbrt::MakeCamera(CameraName, CameraParams, CameraToWorld,
            renderOptions->transformStartTime,
            renderOptions->transformEndTime, film);
        return camera;
    }

    Scene* RenderOptions::MakeScene()
    {
        std::shared_ptr<Primitive> accelerator = MakeAccelerator(AcceleratorName, primitives, AcceleratorParams);
        if(!accelerator) accelerator = std::make_shared<BVHAccel>(primitives);
        Scene* scene = new Scene(accelerator, lights);
        primitives.clear();
        lights.clear();
        return scene;
    }
    Integrator* RenderOptions::MakeIntegrator() const
    {
        std::shared_ptr<const Camera> camera(MakeCamera());
        if (!camera) {
            Error("Unable to create camera");
            return nullptr;
        }
        std::shared_ptr<Sampler> sampler =
            MakeSampler(SamplerName, SamplerParams, camera->film);

        Integrator* integrator = nullptr;

        if (IntegratorName == "path")
            integrator = CreatePathIntegrator(IntegratorParams, sampler, camera);

        IntegratorParams.ReportUnused();
        // Warn if no light sources are defined
        if (lights.empty())
            Warning(
                "No light sources defined in scene; "
                "rendering a black image.");
        return integrator;
    }
}
