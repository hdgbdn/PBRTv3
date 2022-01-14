#ifndef PBRT_CORE_PARSER_H
#define PBRT_CORE_PARSER_H

#include "pbrt.h"

namespace pbrt
{
    // Loc represents a position in a file being parsed.
    struct Loc {
        Loc() = default;
        Loc(const std::string &filename) : filename(filename) {}

        std::string filename;
        int line = 1, column = 0;
    };

    class string_view
    {
    public:
        string_view() = default;
        string_view(const char* start, size_t size):ptr(start), length(size) {}

        const char* data() const { return ptr; }
        size_t size() const { return length; }
        bool empty() const { return length == 0; }

        char operator[](int index) const { return ptr[index]; }
        char back() const { return ptr[length - 1]; }

        const char* begin() const {return ptr; }
        const char* end() const { return ptr + length; }

        bool operator==(const char* str) const
        {
            int index;
            for(index = 0; *str; ++index, ++str)
            {
                if(index >= length) return false;
                if(*str != ptr[index]) return false;
            }
            return index == length;
        }

        bool operator!=(const char* str) const { return !(*this == str); }

        void remove_prefix(int n)
        {
            ptr += n;
            length -=n;
        }

        void remove_suffix(int n)
        {
            length -= n;
        }

        std::string toString()
        {
            return {ptr, length};
        }
    private:
        const char* ptr;
        size_t length;
    };

    class Tokenizer
    {
    public:
        static std::unique_ptr<Tokenizer> CreateFromFile(
                const std::string& fileName,
                std::function<void(const char*)> errorCallback);
        ~Tokenizer();
        string_view Next();
        Loc loc;
    private:
        Tokenizer(std::string str, std::function<void(const char *)> errorCallback);
        int getChar()
        {
            if(pos == end) return EOF;
            char ch = *pos++;
            if(ch == '\n')
            {
                ++loc.line;
                loc.column = 0;
            }
            else
                ++loc.column;
            return ch;
        }
        void ungetChar()
        {
            --pos;
            if(*pos == '\n')
                --loc.line;
        }
        std::function<void(const char *)> errorCallback;
        std::string content;
        const char* pos,* end;
        std::string sEscaped;
    };

	void pbrtParseFile(std::string fileName);
}

#endif