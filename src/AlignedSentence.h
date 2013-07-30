#ifndef ALIGNEDSENTENCE_HDR
#define ALIGNEDSENTENCE_HDR

#include <stdint.h>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "stringpiece.h"

typedef re2::StringPiece StringPiece;

struct Hit
{
    Hit(uint32_t doc, uint8_t start, uint8_t length)
        : doc(doc), start(start), length(length)
    { }

    uint32_t doc;
    uint8_t start;
    uint8_t length;
};

typedef std::vector<Hit> Hits;
typedef boost::shared_ptr<Hits> HitsPtr;

class Phrase;
typedef boost::shared_ptr<Phrase> PhrasePtr;

class Sentence : public boost::enable_shared_from_this<Sentence> {
public:
    Sentence(const std::string&);
    Sentence(const char*);

    const std::string& GetString() const;
    const std::vector<StringPiece>& GetTokens() const;
    const StringPiece& GetToken(size_t) const;
    const std::string GetTokenString(size_t) const;

    const PhrasePtr GetPhrase(size_t, size_t);
    const PhrasePtr AsPhrase();

protected:
    virtual void Init();
    void Tokenize(StringPiece, std::vector<StringPiece>&);

    const std::string m_sentenceString;
    std::vector<StringPiece> m_sentenceTokens;
};

typedef boost::shared_ptr<Sentence> SentencePtr;

class Phrase
{
public:
    Phrase(SentencePtr parentSentence, size_t, size_t);
    std::string ToString() const;
    const std::vector<StringPiece> GetTokens();
    bool operator<(const Phrase& rhs) const;

protected:
    SentencePtr m_parentSentence;
    size_t m_start;
    size_t m_length;
};

bool operator <(const PhrasePtr&, const PhrasePtr&);

typedef std::vector<uint8_t> SingleWordAlignment;
typedef std::vector<SingleWordAlignment> DirectedAlignment;

class TargetPhrase : public Phrase {
public:
    TargetPhrase(SentencePtr parentSentence, size_t, size_t, size_t, size_t);

    const DirectedAlignment GetAlignment() const;
    const std::string GetAlignmentString() const;

private:
    size_t m_sourceStart;
    size_t m_sourceLength;
};

typedef boost::shared_ptr<TargetPhrase> TargetPhrasePtr;
typedef std::vector<TargetPhrasePtr> TargetPhrases;

class AlignedTargetSentence : public Sentence
{
public:
    AlignedTargetSentence(const std::string&, const DirectedAlignment&);
    AlignedTargetSentence(const char*, const DirectedAlignment&);

    // adapted from Moses::BilingualDynSuffixArray
    void ExtractTargetPhrase(TargetPhrases&, size_t, size_t, size_t = 7);
    const DirectedAlignment& GetAlignment() const;

private:
    void Init();

    DirectedAlignment m_alignment;
    std::vector<uint8_t> m_numberAligned;
};


#endif // ALIGNEDSENTENCE_HDR
