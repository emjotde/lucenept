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

typedef std::string StringContainer;
typedef re2::StringPiece StringPiece;
typedef std::stringstream StringStream;

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
    Sentence(const StringContainer&);
    Sentence(const char*);

    const StringContainer& GetString() const;
    const std::vector<StringPiece>& GetTokens() const;
    const StringPiece& GetToken(size_t) const;
    const StringContainer GetTokenString(size_t) const;
    size_t Size();

    const PhrasePtr GetPhrase(size_t, size_t);
    const PhrasePtr AsPhrase();

protected:
    virtual void Init();
    void Tokenize(StringPiece, std::vector<StringPiece>&);

    const StringContainer m_sentenceString;
    std::vector<StringPiece> m_sentenceTokens;
};

typedef boost::shared_ptr<Sentence> SentencePtr;

class Phrase
{
public:
    Phrase(SentencePtr parentSentence, size_t, size_t);
    std::string ToString() const;
    size_t GetStart() const;
    const StringPiece GetStringPiece() const;
    size_t GetLength() const;
    const StringPiece GetToken(size_t) const;
    bool operator<(const Phrase& rhs) const;

protected:
    SentencePtr m_parentSentence;
    size_t m_start;
    size_t m_length;
    StringPiece m_string;
};

bool operator <(const PhrasePtr&, const PhrasePtr&);

typedef std::vector<uint8_t> SingleWordAlignment;
typedef std::vector<SingleWordAlignment> DirectedAlignment;

class TargetPhrase : public Phrase {
public:
    TargetPhrase(SentencePtr parentSentence, size_t, size_t, size_t, size_t);

    const DirectedAlignment GetAlignment() const;
    const StringContainer GetAlignmentString() const;

private:
    size_t m_sourceStart;
    size_t m_sourceLength;
};

typedef boost::shared_ptr<TargetPhrase> TargetPhrasePtr;
typedef std::vector<TargetPhrasePtr> TargetPhrases;

class AlignedTargetSentence : public Sentence
{
public:
    AlignedTargetSentence(const StringContainer&, const DirectedAlignment&);
    AlignedTargetSentence(const char*, const DirectedAlignment&);

    // adapted from Moses::BilingualDynSuffixArray
    void ExtractTargetPhrase(TargetPhrases&, size_t, size_t, size_t = 7);
    const DirectedAlignment& GetAlignment() const;

private:
    void Init();

    DirectedAlignment m_alignment;
    std::vector<uint8_t> m_numberAligned;
};

typedef boost::shared_ptr<AlignedTargetSentence> AlignedTargetSentencePtr;


#endif // ALIGNEDSENTENCE_HDR
