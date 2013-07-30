#include "LucenePT.h"
#include "AlignedSentence.h"
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>

LucenePT::LucenePT(const std::string& dir, bool intoMemory)
    : m_maxSamples(0), index_(new LuceneIndex(dir, intoMemory))
{ }

void LucenePT::createPhrase(const std::string& phrase, size_t maxTargets)
{
    size_t totalCount = 0;
    std::map<TargetPhrase, size_t> phraseCounts;

    HitsPtr hits = index_->GetHits(phrase);
    HitsPtr sample = hits;
    if(m_maxSamples != 0 && sample->size() > m_maxSamples)
        sample = randomSampling(hits->rbegin(), hits->rend(), m_maxSamples);

    BOOST_FOREACH(Hit hit, *sample)
    {
        AlignedSentencePtr as = index_->GetAlignedSentence(hit);
        TargetPhrases targetPhrases;
        as->ExtractTargetPhrase(targetPhrases, hit.start, hit.length);
        totalCount += targetPhrases.size();

        BOOST_FOREACH(TargetPhrase tp, targetPhrases)
            phraseCounts[tp]++;
    }

    typedef std::vector<float> Scores;
    std::multimap<Scores, TargetPhrase> scoreMap;
    BOOST_FOREACH(const TargetPhrase& tp, phraseCounts
                  | boost::adaptors::map_keys)
    {
        Scores scores(2, 0);
        scores[0] = (float)phraseCounts[tp]/totalCount;
        scores[1] = 1;
        scoreMap.insert(std::make_pair(scores, tp));
    }

    size_t targetsCount = 0;
    std::multimap<Scores, TargetPhrase>::const_reverse_iterator revIt;
    for(revIt = scoreMap.rbegin(); revIt != scoreMap.rend(); revIt++)
    {
        std::cout << revIt->second.ToString() << " :";
        BOOST_FOREACH(float score, revIt->first)
            std::cout << " " << score;
        std::cout << std::endl;
        if(targetsCount++ >= maxTargets)
            break;
    }
}
