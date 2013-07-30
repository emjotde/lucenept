#include "AlignedSentence.h"

#include <sstream>

bool operator <(const PhrasePtr& lhs, const PhrasePtr& rhs)
{
    return *lhs < *rhs;
}

Sentence::Sentence(const std::string& trg)
    : m_sentenceString(trg)
{
    Init();
}

Sentence::Sentence(const char* trg)
    : m_sentenceString(trg)
{
    Init();
}

const PhrasePtr Sentence::GetPhrase(size_t start, size_t length)
{
    return PhrasePtr(new Phrase(shared_from_this(), start, length));
}

const PhrasePtr Sentence::AsPhrase()
{
    return PhrasePtr(
        new Phrase(shared_from_this(), 0, m_sentenceTokens.size()));
}

void Sentence::Init()
{
    Tokenize(m_sentenceString, m_sentenceTokens);
}

const std::string& Sentence::GetString() const
{
    return m_sentenceString;
}

const std::vector<StringPiece>& Sentence::GetTokens() const
{
    return m_sentenceTokens;
}

const StringPiece& Sentence::GetToken(size_t i) const
{
    return m_sentenceTokens[i];
}

const std::string Sentence::GetTokenString(size_t i) const
{
    return m_sentenceTokens[i].as_string();
}

void Sentence::Tokenize(StringPiece str,
                        std::vector<StringPiece>& out)
{
    const char* seps = " \t";
    StringPiece suffix = str;
    size_t len = suffix.length();
    size_t span;
    while((span = strcspn(suffix.data(), seps)) != len)
    {
        out.push_back(StringPiece(suffix.data(), span));
        size_t skip = 0;
        char* ptr = (char*)suffix.data() + span;
        while(strpbrk(ptr, seps) == ptr)
        {
            ptr++;
            skip++;
        }
        len = len - span - skip;
        suffix = StringPiece(suffix.data() + span + skip, len);
    }
    if(len)
        out.push_back(suffix);
}

Phrase::Phrase(SentencePtr parentSentence, size_t start, size_t length)
    : m_parentSentence(parentSentence), m_start(start), m_length(length)
{ }

const std::vector<StringPiece> Phrase::GetTokens() {
    std::vector<StringPiece> tokens;
    for(size_t i = m_start; i < m_start + m_length; ++i)
        tokens.push_back(m_parentSentence->GetToken(i));
    return tokens;
}

std::string Phrase::ToString() const
{
    std::stringstream ss;
    for(size_t i = m_start;
            i < m_start + m_length; i++)
    {
        if(i > m_start)
            ss << " ";
        ss << m_parentSentence->GetTokenString(i);
    }
    return ss.str();
}

bool Phrase::operator<(const Phrase& rhs) const
{
    std::vector<StringPiece>::const_iterator
    begin1 = m_parentSentence->GetTokens().begin()
             + m_start,
    end1 = m_parentSentence->GetTokens().begin()
           + m_start + m_length,
    begin2 = rhs.m_parentSentence->GetTokens().begin()
             + rhs.m_start,
    end2 = rhs.m_parentSentence->GetTokens().begin()
           + rhs.m_start + rhs.m_length;

    return lexicographical_compare(begin1, end1, begin2, end2);
}

TargetPhrase::TargetPhrase(SentencePtr parentSentence, size_t start,
                          size_t length, size_t sourceStart, size_t sourceLength)
    : Phrase(parentSentence, start, length), m_sourceStart(sourceStart),
    m_sourceLength(sourceLength)
{ }

const DirectedAlignment TargetPhrase::GetAlignment() const {
    const DirectedAlignment& sentenceAlign
        = boost::shared_static_cast<AlignedTargetSentence>(m_parentSentence)
            ->GetAlignment();

    DirectedAlignment align;
    for(size_t i = m_sourceStart; i < m_sourceStart + m_sourceLength; ++i) {
        BOOST_FOREACH(size_t j, sentenceAlign[i]) {
            if(m_start <= j && j < m_start + m_length) {
                size_t a = i - m_sourceStart; // - ?
                size_t b = j - m_start;

                if(align.size() <= a)
                    align.resize(a + 1);
                align[a].push_back(b);
            }
        }
    }
    return align;
}

const std::string TargetPhrase::GetAlignmentString() const {
    const DirectedAlignment align = GetAlignment();
    std::stringstream ss;
    bool first = true;
    for(size_t i = 0; i < align.size(); ++i) {
        BOOST_FOREACH(size_t j, align[i]) {
            if(!first)
                ss << " ";
            ss << i << "-" << j;
            first = false;
        }
    }
    return ss.str();
}

AlignedTargetSentence::AlignedTargetSentence(const std::string& trg,
                                 const DirectedAlignment& align)
    : Sentence(trg), m_alignment(align)
{
    Init();
}

AlignedTargetSentence::AlignedTargetSentence(const char* trg,
                                 const DirectedAlignment& align)
    : Sentence(trg), m_alignment(align)
{
    Init();
}

// adapted from BilingualDynSuffixArray
void AlignedTargetSentence::ExtractTargetPhrase(TargetPhrases& targetPhrases,
        size_t start, size_t length, size_t maxPhraseLength)
{
    int countTarget = m_numberAligned.size();

    int minTarget = 9999;
    int maxTarget = -1;
    std::vector<uint8_t> usedTarget = m_numberAligned;

    for(size_t sourcePos = start; sourcePos < start + length; sourcePos++)
    {
        if (sourcePos >= m_alignment.size())
            continue;

        for(size_t ind=0; ind < m_alignment[sourcePos].size(); ind++)
        {
            int targetPos = m_alignment[sourcePos][ind];
            if (targetPos<minTarget)
                minTarget = targetPos;
            if (targetPos>maxTarget)
                maxTarget = targetPos;
            usedTarget[targetPos]--;
        }
    }

    if (maxTarget >= 0 && maxTarget - minTarget < (int)maxPhraseLength)
    {
        bool out_of_bounds = false;
        for(int targetPos = minTarget; targetPos <= maxTarget
                && !out_of_bounds; targetPos++)
            if (usedTarget[targetPos] > 0)
                out_of_bounds = true;
        if (!out_of_bounds)
        {
            for(int startTarget = minTarget;
                    (startTarget >= 0 &&
                     startTarget > maxTarget - (int)maxPhraseLength &&
                     (startTarget == minTarget || m_numberAligned[startTarget] == 0));
                    startTarget--)
            {
                for (int endTarget = maxTarget;
                        (endTarget < countTarget &&
                         endTarget < startTarget + (int)maxPhraseLength &&
                         (endTarget == maxTarget || m_numberAligned[endTarget] == 0));
                        endTarget++)
                {
                    TargetPhrasePtr targetPhrase(
                        new TargetPhrase(
                            shared_from_this(),
                            startTarget,
                            endTarget - startTarget + 1,
                            start,
                            length));
                    targetPhrases.push_back(targetPhrase);
                }
            }
        }
    }
}

const DirectedAlignment& AlignedTargetSentence::GetAlignment() const
{
    return m_alignment;
}

void AlignedTargetSentence::Init()
{
    m_numberAligned.resize(m_sentenceTokens.size(), 0);
    BOOST_FOREACH(SingleWordAlignment word, m_alignment)
        BOOST_FOREACH(size_t b, word)
            m_numberAligned[b]++;
}
