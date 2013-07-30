#include "LucenePT.h"
#include "LuceneIndex.h"
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>

LucenePT::LucenePT(const std::string& dir, bool intoMemory)
    : m_maxSamples(300), m_maxTargetPhrases(10),
    m_index(new LuceneIndex(dir, intoMemory))
{ }

void LucenePT::CountTargetPhrases(const PhrasePtr& phrase,
                                  std::map<PhrasePtr, size_t>& phraseCounts,
                                  size_t& totalCount, bool inverse)
{
    HitsPtr hits = m_index->GetHits(phrase, inverse);
    HitsPtr sample = hits;
    if(m_maxSamples != 0 && sample->size() > m_maxSamples)
        // Sample from the back, more recent documents usually come last
        sample = TopNSampling(hits->rbegin(), hits->rend());

    BOOST_FOREACH(Hit hit, *sample)
    {
        TargetPhrases targetPhrases;

        SentencePtr as = m_index->GetAlignedSentence(hit, inverse);
        boost::static_pointer_cast<AlignedTargetSentence>(as)
            ->ExtractTargetPhrase(targetPhrases, hit.start, hit.length);

        totalCount += targetPhrases.size();

        BOOST_FOREACH(TargetPhrasePtr tp, targetPhrases)
            phraseCounts[tp]++;
    }
}

void LucenePT::CreatePhrase(const std::string& phraseString, bool inverse)
{
    size_t totalCount = 0;
    std::map<PhrasePtr, size_t> phraseCounts;

    // Convert phrase string into Phrase with an associated sentence
    // (the same phrase)
    PhrasePtr inputPhrase = SentencePtr(new Sentence(phraseString))->AsPhrase();

    // Collect counts for translation candidates and total count
    CountTargetPhrases(inputPhrase, phraseCounts, totalCount, inverse);

    typedef std::vector<float> Scores;

    // Calculate scores from counts
    std::multimap<Scores, PhrasePtr> scoreMap;
    BOOST_FOREACH(const PhrasePtr tp, phraseCounts
                  | boost::adaptors::map_keys)
    {
        Scores scores;

        //*************************************************************
        // Directed phrase probability P(e|f)
        float pef = (float)phraseCounts[tp] / totalCount;

        // Inverted phrase probability P(f|e) - costly to calculate
        size_t totalCountTarget = 0;
        std::map<PhrasePtr, size_t> phraseCountsTarget;
        // Collect inverse counts for each target phrase
        CountTargetPhrases(tp, phraseCountsTarget,
                          totalCountTarget, !inverse);
        float pfe = (float)phraseCountsTarget[inputPhrase]
            / totalCountTarget;

        // Phrase penalty
        float pen = 1.0;

        scores.push_back(pef);
        scores.push_back(pfe);
        scores.push_back(pen);
        //*************************************************************

        scoreMap.insert(std::make_pair(scores, tp));
    }

    size_t targetsCount = 0;
    std::multimap<Scores, PhrasePtr>::const_reverse_iterator revIt;
    for(revIt = scoreMap.rbegin(); revIt != scoreMap.rend(); revIt++)
    {
        std::cout << phraseString << " ||| "
            << revIt->second->ToString() << " |||";
        BOOST_FOREACH(float score, revIt->first)
            std::cout << " " << score;
        std::cout << " ||| " << boost::shared_static_cast<TargetPhrase>(
            revIt->second)->GetAlignmentString();
        std::cout << std::endl;

        // only output m_maxTargetPhrases
        if(targetsCount++ >= m_maxTargetPhrases)
            break;
    }
}
