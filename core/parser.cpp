#include "parser.h"
#include "stats.h"
#include "memory.h"
#include "spectrum.h"
#include <stdio.h>
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

    constexpr int TokenOptional = 0;
    constexpr int TokenRequired = 1;

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
                std::function<void(const std::string& n, ParamSet p)> apiFunc)
        {

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

        }
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
