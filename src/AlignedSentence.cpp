#include "AlignedSentence.h"

#include <sstream>


TargetPhrase::TargetPhrase(AlignedSentencePtr parentSentence, PhraseSpan phraseSpan)
    : m_parentSentence(parentSentence), m_phraseSpan(phraseSpan)
{ }

std::string TargetPhrase::ToString() const
{
    std::stringstream ss;
    for(size_t i = m_phraseSpan.first;
            i < m_phraseSpan.first + m_phraseSpan.second; i++)
    {
        if(i > m_phraseSpan.first)
            ss << " ";
        ss << m_parentSentence->m_targetTokens[i].as_string();
    }
    return ss.str();
}

bool TargetPhrase::operator<(const TargetPhrase& rhs) const
{
    std::vector<re2::StringPiece>::iterator
    begin1 = m_parentSentence->m_targetTokens.begin()
             + m_phraseSpan.first,
    end1 = m_parentSentence->m_targetTokens.begin()
           + m_phraseSpan.first + m_phraseSpan.second,
    begin2 = rhs.m_parentSentence->m_targetTokens.begin()
             + rhs.m_phraseSpan.first,
    end2 = rhs.m_parentSentence->m_targetTokens.begin()
           + rhs.m_phraseSpan.first
           + rhs.m_phraseSpan.second;

    return lexicographical_compare(begin1, end1, begin2, end2);
}

AlignedSentence::AlignedSentence(const std::string& trg,
                                 const DirectedAlignment& align)
    : m_target(trg), m_alignment(align)
{
    Init();
}

AlignedSentence::AlignedSentence(const char* trg,
                                 const DirectedAlignment& align)
    : m_target(trg), m_alignment(align)
{
    Init();
}

// adapted from BilingualDynSuffixArray
void AlignedSentence::ExtractTargetPhrase(TargetPhrases& targetPhrases,
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
                    TargetPhrase targetPhrase(shared_from_this(),
                                              PhraseSpan(startTarget,
                                              endTarget - startTarget + 1));
                    targetPhrases.push_back(targetPhrase);
                }
            }
        }
    }
}

const std::string& AlignedSentence::GetTarget() const
{
    return m_target;
}

const DirectedAlignment& AlignedSentence::GetAlignment() const
{
    return m_alignment;
}

const std::string AlignedSentence::GetTargetToken(size_t i) const
{
    return m_targetTokens[i].as_string();
}

void AlignedSentence::Init()
{
    Tokenize(m_target, m_targetTokens);
    m_numberAligned.resize(m_targetTokens.size(), 0);
    BOOST_FOREACH(WordAlignment word, m_alignment)
    BOOST_FOREACH(size_t b, word)
    m_numberAligned[b]++;
}

void AlignedSentence::Tokenize(re2::StringPiece str,
                               std::vector<re2::StringPiece>& out)
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
