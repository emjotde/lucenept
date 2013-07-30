#include "LucenePT.h"
#include "AlignedSentence.h"
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>

LucenePT::LucenePT(const std::string& dir, bool intoMemory)
    : m_maxSamples(300), m_maxTargetPhrases(10),
    m_index(new LuceneIndex(dir, intoMemory))
{ }

void LucenePT::countTargetPhrases(const std::string& phrase,
                                  std::map<TargetPhrase, size_t>& phraseCounts,
                                  size_t& totalCount, bool inverse)
{
    HitsPtr hits = m_index->GetHits(phrase, inverse);
    HitsPtr sample = hits;
    if(m_maxSamples != 0 && sample->size() > m_maxSamples)
        // Sample from the back, more recent documents usually come last
        sample = randomSampling(hits->rbegin(), hits->rend());

    BOOST_FOREACH(Hit hit, *sample)
    {
        AlignedSentencePtr as = m_index->GetAlignedSentence(hit, inverse);
        TargetPhrases targetPhrases;
        as->ExtractTargetPhrase(targetPhrases, hit.start, hit.length);
        totalCount += targetPhrases.size();

        BOOST_FOREACH(TargetPhrase tp, targetPhrases)
            phraseCounts[tp]++;
    }
}

void LucenePT::createPhrase(const std::string& phrase, bool inverse)
{
    size_t totalCount = 0;
    std::map<TargetPhrase, size_t> phraseCounts;

    countTargetPhrases(phrase, phraseCounts, totalCount, inverse);

    typedef std::vector<float> Scores;
    std::multimap<Scores, TargetPhrase> scoreMap;
    BOOST_FOREACH(const TargetPhrase& tp, phraseCounts
                  | boost::adaptors::map_keys)
    {
        Scores scores;
        // Directed phrase probability P(e|f)
        scores.push_back(totalCount);
        scores.push_back(phraseCounts[tp]);
        scores.push_back((float)phraseCounts[tp]/totalCount);

        // Inverted phrase probability P(f|e)
//        size_t totalCountTarget = 0;
//        totalCountTarget = m_index->GetHits(tp.ToString(), !inverse)->size();
//        std::map<TargetPhrase, size_t> phraseCountsTarget;
//        countTargetPhrases(tp.ToString(), phraseCountsTarget,
//                          totalCountTarget, !inverse);
//        scores.push_back(totalCountTarget);
//        scores.push_back((float)phraseCounts[tp]/totalCountTarget);

        // Phrase penalty
        scores.push_back(1);
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
        if(targetsCount++ >= m_maxTargetPhrases)
            break;
    }
}
