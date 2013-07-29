#ifndef ALIGNEDSENTENCE_HDR
#define ALIGNEDSENTENCE_HDR

#include <cstring>
#include <vector>
#include <string>

#include "stringpiece.h"

typedef std::pair<uint8_t, uint8_t> AlignPoint;
typedef std::vector<AlignPoint> Alignment;

class AlignedSentence
{
public:
    AlignedSentence(const std::string& src,
                    const std::string& trg,
                    const Alignment& align)
        : source(src), target(trg), alignment(align)
    {
        init();
    }

    AlignedSentence(const char* src,
                    const char* trg,
                    const Alignment& align)
        : source(src), target(trg), alignment(align)
    {
        init();
    }

    const std::string& getSource() const
    {
        return source;
    }

    const std::string& getTarget() const
    {
        return target;
    }

    const Alignment& getAlignment() const
    {
        return alignment;
    }

    size_t getSourceLength() const
    {
        return sourceTokens.size();
    }

    size_t getTargetLength() const
    {
        return targetTokens.size();
    }

    const std::string getSourceToken(size_t i) const
    {
        return sourceTokens[i].as_string();
    }

    const std::string getTargetToken(size_t i) const
    {
        return targetTokens[i].as_string();
    }

private:
    void init()
    {
        tokenize(source, sourceTokens);
        tokenize(target, targetTokens);
    }

    void tokenize(re2::StringPiece str, std::vector<re2::StringPiece>& out)
    {
        const char* seps = " \t";
        re2::StringPiece suffix = str;
        size_t len = suffix.length();
        size_t span;
        while((span = strcspn(suffix.data(), seps)) != len)
        {
            out.push_back(re2::StringPiece(suffix.data(), span));
            size_t skip = 0;
            char* ptr = (char*)suffix.data() + span;
            while(strpbrk(ptr, seps) == ptr)
            {
                ptr++;
                skip++;
            }
            len = len - span - skip;
            suffix = re2::StringPiece(suffix.data() + span + skip, len);
        }
        if(len)
            out.push_back(suffix);
    }

    const std::string source;
    const std::string target;
    std::vector<re2::StringPiece> sourceTokens;
    std::vector<re2::StringPiece> targetTokens;
    Alignment alignment;
};

#endif // ALIGNEDSENTENCE_HDR
