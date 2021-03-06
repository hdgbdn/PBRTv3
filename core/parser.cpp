#include "parser.h"
#include "api.h"
#include "stats.h"
#include "memory.h"
#include "spectrum.h"
#include "paramset.h"
#include <cstdio>
#include <functional>
#include "fileutil.h"

namespace pbrt
{
    Loc *parserLoc;

    STAT_MEMORY_COUNTER("Memory/Tokenizer buffers", tokenizerMemory);

    static char decodeEscaped(int ch) {
        switch (ch) {
            case EOF:
                Error("premature EOF after character escape '\\'");
                exit(1);
            case 'b':
                return '\b';
            case 'f':
                return '\f';
            case 'n':
                return '\n';
            case 'r':
                return '\r';
            case 't':
                return '\t';
            case '\\':
                return '\\';
            case '\'':
                return '\'';
            case '\"':
                return '\"';
            default:
                Error("unexpected escaped character \"%c\"", ch);
                exit(1);
        }
        return 0;  // NOTREACHED
    }

    std::unique_ptr<Tokenizer>
    Tokenizer::CreateFromFile(const std::string &fileName, std::function<void(const char *)> errorCallback)
    {
        if(fileName == "-")
        {

        }
        FILE* f = fopen(fileName.c_str(), "r");
        if(!f)  Error("Read failed");

        std::string str;
        int ch;
        while((ch = fgetc(f)) != EOF) str.push_back(char(ch));
        fclose(f);
        return std::unique_ptr<Tokenizer>(
                new Tokenizer(std::move(str), std::move(errorCallback)));
    }

    Tokenizer::Tokenizer(std::string str, std::function<void(const char *)> errorCallback)
    : loc("<stdin>"), errorCallback(std::move(errorCallback)), content(std::move(str))
    {
        pos = content.data();
        end = pos + content.size();
        tokenizerMemory += content.size();
    }

    Tokenizer::~Tokenizer()
    {

    }

    string_view Tokenizer::Next()
    {
        while(true)
        {
            const char* tokenStart = pos;
            int ch = getChar();
            if(ch == EOF) return {};
            else if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r'){}
            else if (ch == '"')
            {
                bool haveEscaped = false;
                while((ch = getChar()) != '"')
                {
                    if(ch == EOF)
                    {
                        errorCallback("premature EOF");
                        return {};
                    }
                    else if(ch == '\n')
                    {
                        errorCallback("unterminated string");
                        return {};
                    }
                    else if(ch == '\\')
                    {
                        // wrapped line
                        haveEscaped = true;
                        if((ch = getChar()) == EOF)
                        {
                            errorCallback("premature EOF");
                            return {};
                        }
                    }
                }

                if(!haveEscaped)
                    // now pos is one position behind the last '"'
                    return {tokenStart, size_t(pos - tokenStart)};
                else
                {
                    sEscaped.clear();
                    for (const char* p = tokenStart; p < pos; ++p)
                    {
                        if(*p != '\\')
                            sEscaped.push_back(*p);
                        else
                        {
                            ++p;
                            //CHECK_LT(p, pos);
                            sEscaped.push_back(decodeEscaped(*p));
                        }
                    }
                    return { sEscaped.data(), sEscaped.size() };
                }
            }
            else if (ch == '[' || ch == ']')
                return {tokenStart, size_t(1) };
            else if (ch == '#')
            {
                // comment: scan to EOL (or EOF)
                while((ch = getChar()) != EOF)
                {
                    if(ch == '\n' || ch == '\r')
                    {
                        // now pos is pointed to \n or \r
                        // so next time getChar() will get char next to pos
                        ungetChar();
                        break;
                    }
                }
                return {tokenStart, size_t(pos - tokenStart)};
            }
            else
            {
                // Regular statement or numeric token; scan until we hit a
                // space, opening quote, or bracket.
                while ((ch = getChar()) != EOF) {
                    if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' ||
                        ch == '"' || ch == '[' || ch == ']') {
                        ungetChar();
                        break;
                    }
                }
                return {tokenStart, size_t(pos - tokenStart)};
            }
        }
    }

    static double parseNumber(string_view str)
    {
        if (str.size() == 1) {
            if (!(str[0] >= '0' && str[0] <= '9')) {
                Error("\"%c\": expected a number", str[0]);
                exit(1);
            }
            return str[0] - '0';
        }

        char buf[64];
        char *bufp = buf;
        std::unique_ptr<char[]> allocBuf;
        if (str.size() + 1 >= sizeof(buf))
        {
            allocBuf.reset(new char[str.size() + 1]);
            bufp = allocBuf.get();
        }
        std::copy(str.begin(), str.end(), bufp);
        bufp[str.size()] = '\0';

        auto isInteger = [](string_view str)
        {
            for (char ch : str)
                if (!(ch >= '0' && ch <= '9')) return false;
            return true;
        };

        char* endPtr = nullptr;
        double val;
        if (isInteger(str))
            val = double(strtol(bufp, &endPtr, 10));
        else
            val = strtof(bufp, &endPtr);

        // not a number
        if(val == 0 && endPtr == bufp)
        {
            Error("%s: expected a number", str.toString().c_str());
            exit(1);
        }

        return val;
    }

    inline bool isQuotedString(string_view str) {
        return str.size() >= 2 && str[0] == '"' && str.back() == '"';
    }

    static string_view dequoteString(string_view str) {
        if (!isQuotedString(str)) {
            Error("\"%s\": expected quoted string", str.toString().c_str());
            exit(1);
        }

        str.remove_prefix(1);
        str.remove_suffix(1);
        return str;
    }

    struct ParamListItem {
        std::string name;
        double *doubleValues = nullptr;
        const char **stringValues = nullptr;
        size_t size = 0;
        bool isString = false;
    };

    constexpr int TokenOptional = 0;
    constexpr int TokenRequired = 1;

    enum {
        PARAM_TYPE_INT,
        PARAM_TYPE_BOOL,
        PARAM_TYPE_FLOAT,
        PARAM_TYPE_POINT2,
        PARAM_TYPE_VECTOR2,
        PARAM_TYPE_POINT3,
        PARAM_TYPE_VECTOR3,
        PARAM_TYPE_NORMAL,
        PARAM_TYPE_RGB,
        PARAM_TYPE_XYZ,
        PARAM_TYPE_BLACKBODY,
        PARAM_TYPE_SPECTRUM,
        PARAM_TYPE_STRING,
        PARAM_TYPE_TEXTURE
    };

    static bool lookupType(const std::string& decl, int* type, std::string& sname)
    {
        *type = 0;

        auto skipSpace = [&decl](std::string::const_iterator iter) {
            while (iter != decl.end() && (*iter == ' ' || *iter == '\t')) ++iter;
            return iter;
        };

        auto skipToSpace = [&decl](std::string::const_iterator iter) {
            while (iter != decl.end() && *iter != ' ' && *iter != '\t') ++iter;
            return iter;
        };

        auto typeBegin = skipSpace(decl.begin());
        if (typeBegin == decl.end()) {
            Error("Parameter \"%s\" doesn't have a type declaration?!",
                  decl.c_str());
            return false;
        }

        auto typeEnd = skipToSpace(typeBegin);
        string_view typeStr(&(*typeBegin), size_t(typeEnd - typeBegin));
        if (typeStr == "float")
            *type = PARAM_TYPE_FLOAT;
        else if (typeStr == "integer")
            *type = PARAM_TYPE_INT;
        else if (typeStr == "bool")
            *type = PARAM_TYPE_BOOL;
        else if (typeStr == "point2")
            *type = PARAM_TYPE_POINT2;
        else if (typeStr == "vector2")
            *type = PARAM_TYPE_VECTOR2;
        else if (typeStr == "point3")
            *type = PARAM_TYPE_POINT3;
        else if (typeStr == "vector3")
            *type = PARAM_TYPE_VECTOR3;
        else if (typeStr == "point")
            *type = PARAM_TYPE_POINT3;
        else if (typeStr == "vector")
            *type = PARAM_TYPE_VECTOR3;
        else if (typeStr == "normal")
            *type = PARAM_TYPE_NORMAL;
        else if (typeStr == "string")
            *type = PARAM_TYPE_STRING;
        else if (typeStr == "texture")
            *type = PARAM_TYPE_TEXTURE;
        else if (typeStr == "color")
            *type = PARAM_TYPE_RGB;
        else if (typeStr == "rgb")
            *type = PARAM_TYPE_RGB;
        else if (typeStr == "xyz")
            *type = PARAM_TYPE_XYZ;
        else if (typeStr == "blackbody")
            *type = PARAM_TYPE_BLACKBODY;
        else if (typeStr == "spectrum")
            *type = PARAM_TYPE_SPECTRUM;
        else {
            Error("Unable to decode type from \"%s\"", decl.c_str());
            return false;
        }
        auto nameBegin = skipSpace(typeEnd);
        if (nameBegin == decl.end()) {
            Error("Unable to find parameter name from \"%s\"", decl.c_str());
            return false;
        }
        auto nameEnd = skipToSpace(nameBegin);
        sname = std::string(nameBegin, nameEnd);

        return true;
    }

    static const char *paramTypeToName(int type) {
        switch (type) {
            case PARAM_TYPE_INT:
                return "int";
            case PARAM_TYPE_BOOL:
                return "bool";
            case PARAM_TYPE_FLOAT:
                return "float";
            case PARAM_TYPE_POINT2:
                return "point2";
            case PARAM_TYPE_VECTOR2:
                return "vector2";
            case PARAM_TYPE_POINT3:
                return "point3";
            case PARAM_TYPE_VECTOR3:
                return "vector3";
            case PARAM_TYPE_NORMAL:
                return "normal";
            case PARAM_TYPE_RGB:
                return "rgb/color";
            case PARAM_TYPE_XYZ:
                return "xyz";
            case PARAM_TYPE_BLACKBODY:
                return "blackbody";
            case PARAM_TYPE_SPECTRUM:
                return "spectrum";
            case PARAM_TYPE_STRING:
                return "string";
            case PARAM_TYPE_TEXTURE:
                return "texture";
            default:
                Error("Error in paramTypeToName");
                return nullptr;
        }
    }

    static void AddParam(ParamSet &ps, const ParamListItem &item,
                         SpectrumType spectrumType)
    {
        int type;
        std::string name;
        if(lookupType(item.name, &type, name))
        {
            if(type == PARAM_TYPE_TEXTURE || type == PARAM_TYPE_STRING ||
               type == PARAM_TYPE_BOOL)
            {
                if (!item.stringValues) {
                    Error(
                            "Expected string parameter value for parameter "
                            "\"%s\" with type \"%s\". Ignoring.",
                            name.c_str(), paramTypeToName(type));
                    return;
                }
            }
            else if (type != PARAM_TYPE_SPECTRUM)
            {
                if (item.stringValues)
                {
                    Error(
                            "Expected numeric parameter value for parameter "
                            "\"%s\" with type \"%s\".  Ignoring.",
                            name.c_str(), paramTypeToName(type));
                    return;
                }
            }

            int nItems = item.size;
            if (type == PARAM_TYPE_INT)
            {
                int nAlloc = nItems;
                std::unique_ptr<int[]> idata(new int[nAlloc]);
                for (int j = 0; j < nAlloc; ++j)
                    idata[j] = static_cast<int>(item.doubleValues[j]);
                ps.AddInt(name, std::move(idata), nItems);
            }
            else if (type == PARAM_TYPE_FLOAT)
            {
                int nAlloc = nItems;
                std::unique_ptr<float []> fdata(new float[nAlloc]);
                for (int j = 0; j < nAlloc; ++j)
                    fdata[j] = static_cast<float>(item.doubleValues[j]);
                ps.AddFloat(name, std::move(fdata), nItems);
            }
            else if (type == PARAM_TYPE_POINT3)
            {
                if ((nItems % 3) != 0)
                    Warning(
                            "Excess values given with point3 parameter \"%s\". "
                            "Ignoring last %d of them.",
                            item.name.c_str(), nItems % 3);
                int nAlloc = nItems;
                std::unique_ptr<Point3f[]> pData(new Point3f[nItems / 3]);
                for (int i = 0; i < nItems / 3; ++i) {
                    pData[i].x = static_cast<float>(item.doubleValues[3 * i]);
                    pData[i].y = static_cast<float>(item.doubleValues[3 * i + 1]);
                    pData[i].z = static_cast<float>(item.doubleValues[3 * i + 2]);
                }
                ps.AddPoint3f(name, std::move(pData), nItems / 3);
            }
            else if (type == PARAM_TYPE_NORMAL)
            {
                if ((nItems % 3) != 0)
                    Warning(
                        "Excess values given with normal3 parameter \"%s\". "
                        "Ignoring last %d of them.",
                        item.name.c_str(), nItems % 3);
                int nAlloc = nItems;
                std::unique_ptr<Normal3f[]> pData(new Normal3f[nItems / 3]);
                for (int i = 0; i < nItems / 3; ++i) {
                    pData[i].x = static_cast<float>(item.doubleValues[3 * i]);
                    pData[i].y = static_cast<float>(item.doubleValues[3 * i + 1]);
                    pData[i].z = static_cast<float>(item.doubleValues[3 * i + 2]);
                }
                ps.AddNormal3f(name, std::move(pData), nItems / 3);
            }
            else if (type == PARAM_TYPE_STRING)
            {
                std::unique_ptr<std::string[]> strings(new std::string[nItems]);
                for(int j = 0; j < nItems; ++j)
                    strings[j] = std::string(item.stringValues[j]);
                ps.AddString(name, std::move(strings), nItems);
            }
            else if (type == PARAM_TYPE_RGB)
            {
                if ((nItems % 3) != 0)
                {
                    Warning(
                            "Excess RGB values given with parameter \"%s\". "
                            "Ignoring last %d of them",
                            item.name.c_str(), nItems % 3);
                    nItems -= nItems % 3;
                }
                std::unique_ptr<float[]> floats(new float[nItems]);
                for(int j = 0; j < nItems; ++j) floats[j] = static_cast<float>(item.doubleValues[j]);
                ps.AddRGBSpectrum(name, std::move(floats), nItems);
            }
            else if (type == PARAM_TYPE_TEXTURE) 
            {
                if (nItems == 1) {
                    std::string val(*item.stringValues);
                    ps.AddTexture(name, val);
                }
                else
                    Error(
                        "Only one string allowed for \"texture\" parameter "
                        "\"%s\"",
                        name.c_str());
            }
            else
                Warning("Type of parameter {} is unknown", item.name.c_str());
        }
    }

    template <typename Next, typename Unget>
    ParamSet parseParams(Next nextToken, Unget ungetToken, MemoryArena &arena,
                         SpectrumType spectrumType)
    {
        ParamSet ps;
        while(true)
        {
            // get token name
            string_view decl = nextToken(TokenOptional);
            if(decl.empty()) return ps;

            // quit if reach the next unquoted token(means the next attribute)
            if(!isQuotedString(decl))
            {
                ungetToken(decl);
                return ps;
            }

            ParamListItem item;
            item.name = dequoteString(decl).toString();
            size_t nAlloc = 0;

            // like "integer pixelsamples" [ 2048 ] here
            auto addVal = [&](string_view val)
            {
                if(isQuotedString(val))
                {
                    // string values enter here
                    // so shouldn't contain any double numbers
                    if(item.doubleValues)
                    {
                        Error("mixed string and numeric parameters");
                        exit(1);
                    }
                    if(item.size == nAlloc)
                    {
                        // if previous allocated memory isn't enough
                        // then alloca a new arena for the stringValues and keep the
                        // origin data
                        nAlloc = std::max<size_t>(2 * item.size, 4);
                        const char** newData = arena.Alloc<const char* >(nAlloc);
                        std::copy(item.stringValues, item.stringValues + item.size,
                                  newData);
                        item.stringValues = newData;
                    }

                    // append next string value
                    val = dequoteString(val);
                    char* buf = arena.Alloc<char>(val.size() + 1);
                    memcpy(buf, val.data(), val.size());
                    buf[val.size()] = '\0';
                    item.stringValues[item.size++] = buf;
                }
                else
                {
                    // double number values enter here
                    // so shouldn't contain any string values
                    if(item.stringValues)
                    {
                        Error("mixed string and numeric parameters");
                        exit(1);
                    }

                    if(item.size == nAlloc)
                    {
                        nAlloc = std::max<size_t>(2 * item.size, 4);
                        double* newData = arena.Alloc<double>(nAlloc);
                        std::copy(item.doubleValues, item.doubleValues + item.size,
                                  newData);
                        item.doubleValues = newData;
                    }
                    item.doubleValues[item.size++] = parseNumber(val);
                }
            };

            // get param values
            string_view val = nextToken(TokenRequired);

            if(val == "[")
            {
                while (true)
                {
                    // add values until meet "]"
                    val = nextToken(TokenRequired);
                    if (val == "]") break;
                    addVal(val);
                }
            }
            else
            {
                addVal(val);
            }
            AddParam(ps, item, spectrumType);
            // now data are all copied into paramset, so clear the arena
            arena.Reset();
        }
        return ps;
    }

    static void parse(std::unique_ptr<Tokenizer> t)
    {
        std::vector<std::unique_ptr<Tokenizer>> fileStack;
        fileStack.emplace_back(std::move(t));
        parserLoc = &fileStack.back()->loc;

        bool ungetTokenSet = false;
        std::string ungetTokenValue;

        // return the next token from current file, until reach EOF,
        // at which point it switches to the next file(if exits)
        std::function<string_view(int)> nextToken;
        nextToken = [&](int flags) -> string_view
        {
            if (ungetTokenSet)
            {
                ungetTokenSet = false;
                return {ungetTokenValue.data(), ungetTokenValue.size()};
            }
            if(fileStack.empty())
            {
                if(flags & TokenRequired)
                {
                    Error("premature EOF");
                    exit(1);
                }
                parserLoc = nullptr;
                return {};
            }

            string_view tok = fileStack.back()->Next();
            if(tok.empty())
            {
                fileStack.pop_back();
                if(!fileStack.empty()) parserLoc = &fileStack.back()->loc;
                return nextToken(flags);
            }
            else if(tok[0] == '#')
            {
                // Swallow comments
                return nextToken(flags);
            }
            else return tok;
        };

        auto ungetToken = [&](string_view s)
        {
            // CHECK(!ungetTokenSet);
            ungetTokenValue = std::string(s.data(), s.size());
            ungetTokenSet = true;
        };

        MemoryArena arena;

        auto basicParamListEntrypoint = [&](
                SpectrumType spectrumType,
                std::function<void(const std::string &n, ParamSet p)> apiFunc)
        {
            string_view token = nextToken(TokenRequired);
            string_view dequoted = dequoteString(token);
            std::string n = dequoted.toString();
            ParamSet params =
                    parseParams(nextToken, ungetToken, arena, spectrumType);
            apiFunc(n, std::move(params));
        };

        auto syntaxError = [&](string_view tok)
        {
            Error("Unexpected token: %s", tok.toString().c_str());
            exit(1);
        };

        while(true)
        {
            string_view tok = nextToken(TokenOptional);
            if(tok.empty()) break;
            switch (tok[0])
            {
                case 'A':
                    if (tok == "AttributeBegin")
                        pbrtAttributeBegin();
                    else if (tok == "AttributeEnd")
                        pbrtAttributeEnd();
                    else if (tok == "ActiveTransform") {
                        string_view a = nextToken(TokenRequired);
                        if (a == "All")
                            pbrtActiveTransformAll();
                        else if (a == "EndTime")
                            pbrtActiveTransformEndTime();
                        else if (a == "StartTime")
                            pbrtActiveTransformStartTime();
                        else
                            syntaxError(tok);
                    } else if (tok == "AreaLightSource")
                        basicParamListEntrypoint(SpectrumType::Illuminant,
                                                 pbrtAreaLightSource);
                    else if (tok == "Accelerator")
                        basicParamListEntrypoint(SpectrumType::Reflectance,
                                                 pbrtAccelerator);
                    else
                        syntaxError(tok);
                    break;
                case 'C':
                    if (tok == "ConcatTransform") {
                        if (nextToken(TokenRequired) != "[") syntaxError(tok);
                        float m[16];
                        for (int i = 0; i < 16; ++i)
                            m[i] = parseNumber(nextToken(TokenRequired));
                        if (nextToken(TokenRequired) != "]") syntaxError(tok);
                        pbrtConcatTransform(m);
                    }
                    else if (tok == "Camera")
                        basicParamListEntrypoint(SpectrumType::Reflectance,
                                                 pbrtCamera);
            	else if (tok == "CoordSysTransform") {
                        string_view n = dequoteString(nextToken(TokenRequired));
                        pbrtCoordSysTransform(n.toString());
                    }
                    else
                    syntaxError(tok);
                    break;
                case 'F':
                    if (tok == "Film")
                        basicParamListEntrypoint(SpectrumType::Reflectance, pbrtFilm);
                    else
                        syntaxError(tok);
                    break;
                case 'I':
                    if (tok == "Integrator")
                    {
                        basicParamListEntrypoint(SpectrumType::Reflectance, pbrtIntegrator);
                    }
                    else if (tok == "Include")
                    {
                        std::string fileName = dequoteString(nextToken(TokenRequired)).toString();
                        fileName = AbsolutePath(ResolveFilename(fileName));
                        auto tokError = [](const char *msg) { Error("%s", msg); exit(1); };
                        std::unique_ptr<Tokenizer> tinc = Tokenizer::CreateFromFile(fileName, tokError);
                        if (nullptr != tinc)
                        {
                            fileStack.push_back(std::move(tinc));
                            parserLoc = &fileStack.back()->loc;
                        }
                    }

                    break;
                case 'L':
                    if (tok == "LightSource")
                        basicParamListEntrypoint(SpectrumType::Illuminant,
                                                 pbrtLightSource);
                    else if (tok == "LookAt")
                    {
                        float v[9];
                        for(int i = 0; i < 9; ++i)
                            v[i] = parseNumber(nextToken(TokenRequired));
                        pbrtLookAt(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
                                   v[8]);
                    }
                    else
                        syntaxError(tok);
                    break;
                case 'M':
                    if (tok == "Material")
                        basicParamListEntrypoint(SpectrumType::Reflectance,
                                                 pbrtMaterial);
                    break;
                case 'N':
                    break;
                case 'O':
                    break;
                case 'P':
                    if (tok == "PixelFilter")
                        basicParamListEntrypoint(SpectrumType::Reflectance,
                                                 pbrtPixelFilter);
                    else
                        syntaxError(tok);
                    break;
                case 'R':
                    if (tok == "Rotate")
                    {
                        float v[4];
                        for (int i = 0; i < 4; ++i)
                            v[i] = parseNumber(nextToken(TokenRequired));
                        pbrtRotate(v[0], v[1], v[2], v[3]);
                    }
                    break;
                case 'S':
                    if (tok == "Shape")
                        basicParamListEntrypoint(SpectrumType::Reflectance, pbrtShape);
                    else if (tok == "Sampler")
                        basicParamListEntrypoint(SpectrumType::Reflectance,
                                                 pbrtSampler);
                    else if (tok == "Scale") {
                        float v[3];
                        for (int i = 0; i < 3; ++i)
                            v[i] = parseNumber(nextToken(TokenRequired));
                        pbrtScale(v[0], v[1], v[2]);
                    } else
                        syntaxError(tok);
                    break;
                case 'T':
                    if (tok == "TransformBegin")
                        pbrtTransformBegin();
                    else if (tok == "TransformEnd")
                        pbrtTransformEnd();
                    else if (tok == "Transform")
                    {
                        if (nextToken(TokenRequired) != "[") syntaxError(tok);
                        float m[16];
                        for (int i = 0; i < 16; ++i)
                            m[i] = parseNumber(nextToken(TokenRequired));
                        if (nextToken(TokenRequired) != "]") syntaxError(tok);
                        pbrtTransform(m);
                    }
                    else if (tok == "Translate")
                    {
                        float v[3];
                        for (int i = 0; i < 3; ++i)
                            v[i] = parseNumber(nextToken(TokenRequired));
                        pbrtTranslate(v[0], v[1], v[2]);
                    }
                    else if (tok == "Texture")
                    {
                        string_view n = dequoteString(nextToken(TokenRequired));
                        std::string name = n.toString();
                        n = dequoteString(nextToken(TokenRequired));
                        std::string type = n.toString();

                        basicParamListEntrypoint(
                                SpectrumType::Reflectance,
                                [&](const std::string &texName, const ParamSet &params) {
                                    pbrtTexture(name, type, texName, params);
                                });
                    }
                    break;
                case 'W':
                    if (tok == "WorldBegin")
                        pbrtWorldBegin();
                    else if (tok == "WorldEnd")
                        pbrtWorldEnd();
                    else
                        syntaxError(tok);
                    break;
                default:
                    syntaxError(tok);
            }
        }
    }

    void pbrtParseFile(std::string fileName)
    {
        if(fileName != "-")
        {
            // TODO handle stdin
        }
        auto tokError = [](const char *msg) { Error("%s", msg); exit(1); };
        std::unique_ptr<Tokenizer> t =
            Tokenizer::CreateFromFile(fileName, tokError);
        if(!t) return;
        parse(std::move(t));
    }
}
