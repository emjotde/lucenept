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

typedef std::vector<uint8_t> WordAlignment;
typedef std::vector<WordAlignment> DirectedAlignment;

class AlignedSentence;
typedef boost::shared_ptr<AlignedSentence> AlignedSentencePtr;
typedef std::pair<uint8_t, uint8_t> PhraseSpan;

class TargetPhrase
{
public:
    TargetPhrase(AlignedSentencePtr parentSentence, PhraseSpan phraseSpan);
    std::string ToString() const;
    bool operator<(const TargetPhrase& rhs) const;

private:
    AlignedSentencePtr m_parentSentence;
    PhraseSpan m_phraseSpan;
};

typedef std::vector<TargetPhrase> TargetPhrases;

class AlignedSentence : public boost::enable_shared_from_this<AlignedSentence>
{
    friend TargetPhrase;

public:
    AlignedSentence(const std::string&, const DirectedAlignment&);
    AlignedSentence(const char*, const DirectedAlignment&);

    // adapted from Moses::BilingualDynSuffixArray
    void ExtractTargetPhrase(TargetPhrases&, size_t, size_t, size_t = 7);
    const std::string& GetTarget() const;
    const DirectedAlignment& GetAlignment() const;
    const std::string GetTargetToken(size_t) const;

private:
    void Init();
    void Tokenize(re2::StringPiece, std::vector<re2::StringPiece>&);

    const std::string m_target;
    std::vector<re2::StringPiece> m_targetTokens;
    DirectedAlignment m_alignment;
    std::vector<uint8_t> m_numberAligned;
};


#endif // ALIGNEDSENTENCE_HDR
