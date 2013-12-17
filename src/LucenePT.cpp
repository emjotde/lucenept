#include "LucenePT.h"
#include "LuceneIndex.h"
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>

LucenePT::LucenePT(const std::string& dir, bool intoMemory)
    : m_maxSamples(100), m_maxTargetPhrases(30), m_maxPhraseLength(7),
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
        sample = RandomSampling(hits->rbegin(), hits->rend());

    BOOST_FOREACH(Hit hit, *sample)
    {
        TargetPhrases targetPhrases;

        SentencePtr as = m_index->GetAlignedSentence(hit, inverse);
        boost::static_pointer_cast<AlignedTargetSentence>(as)
            ->ExtractTargetPhrase(targetPhrases, hit.start, hit.length,
                                  m_maxPhraseLength);

        totalCount += targetPhrases.size();

        BOOST_FOREACH(TargetPhrasePtr tp, targetPhrases)
            phraseCounts[tp]++;
    }
}

std::set<PhrasePtr> skip;

void LucenePT::AllPhrases(const std::string& sentenceString, bool inverse)
{
    SentencePtr sentence(new Sentence(sentenceString));
    for(size_t start = 0; start < sentence->Size(); ++start)
    {
        for(size_t length = 1; length < m_maxPhraseLength
            && start + length <= sentence->Size(); ++length)
        {
            PhrasePtr phrase = sentence->GetPhrase(start, length);
            //std::cout << phrase->ToString() << std::endl;
            if(skip.count(phrase) == 0) {
                CreatePhrase(phrase, inverse);
                skip.insert(phrase);
            }
            else {
                //std::cout << "Skipped" << std::endl;
            }
        }
    }
}

void LucenePT::CreatePhrase(const std::string& phraseString, bool inverse)
{
    // Convert phrase string into Phrase with an associated sentence
    // (actually the same phrase)
    PhrasePtr inputPhrase = SentencePtr(new Sentence(phraseString))->AsPhrase();
    CreatePhrase(inputPhrase, inverse);
}

void LucenePT::CreatePhrase(const PhrasePtr& inputPhrase, bool inverse)
{
    size_t totalCount = 0;
    std::map<PhrasePtr, size_t> phraseCounts;

    if(inputPhrase->GetLength() > m_maxPhraseLength)
        return;

    // Collect counts for translation candidates and total count
    CountTargetPhrases(inputPhrase, phraseCounts, totalCount, inverse);

    typedef std::vector<double> Scores;

    //HitsPtr srcHits = m_index->GetHits(inputPhrase, inverse);
    //std::vector<int> srcIds;
    //BOOST_FOREACH(Hit h, *srcHits)
    //    if(srcIds.empty() || srcIds.back() != h.doc)
    //        srcIds.push_back(h.doc);

    // Calculate scores from counts
    std::multimap<Scores, PhrasePtr> scoreMap;
    BOOST_FOREACH(const PhrasePtr tp, phraseCounts
                  | boost::adaptors::map_keys)
    {
        Scores scores;

        //HitsPtr trgHits = m_index->GetHits(tp, !inverse);
        //std::vector<int> trgIds;
        //BOOST_FOREACH(Hit h, *trgHits)
        //    if(trgIds.empty() || trgIds.back() != h.doc)
        //        trgIds.push_back(h.doc);

        //std::vector<int> commonIds;
        //std::set_intersection(srcIds.begin(), srcIds.end(), trgIds.begin(),
        //                      trgIds.end(), std::back_inserter(commonIds));

        //int cfe = commonIds.size();
        //int cf = srcIds.size();
        //int ce = trgIds.size();
        //int N = m_index->Size();
        //
        //double pv = -log(fisher_exact(cfe, cf, ce, N));

        //if(pv < 20)
        //    continue;

        //*************************************************************
        // Directed phrase probability rowP(e|f)
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

//        scores.push_back(cfe);
//        scores.push_back(cf);
//        scores.push_back(ce);
//        scores.push_back(N);
//        scores.push_back(pv);

        //*************************************************************

        scoreMap.insert(std::make_pair(scores, tp));
    }

    size_t targetsCount = 0;
    std::multimap<Scores, PhrasePtr>::const_reverse_iterator revIt;
    for(revIt = scoreMap.rbegin(); revIt != scoreMap.rend(); revIt++)
    {
        const Scores& scores = revIt->first;
        TargetPhrasePtr targetPhrase
            = boost::static_pointer_cast<TargetPhrase>(revIt->second);

        std::cout << inputPhrase->ToString() << " ||| "
            << targetPhrase->ToString() << " |||";
        BOOST_FOREACH(float score, scores)
            std::cout << " " << score;
        std::cout << " ||| " << targetPhrase->GetAlignmentString();
        std::cout << std::endl;

        // only output m_maxTargetPhrases
        if(targetsCount++ >= m_maxTargetPhrases)
            break;
    }
}

// 2x2 (one-sided) Fisher's exact test
// see B. Moore. (2004) On Log Likelihood and the Significance of Rare Events
double fisher_exact(int cfe, int ce, int cf, int num)
{
    assert(cfe <= ce);
    assert(cfe <= cf);

    int a = cfe;
    int b = (cf - cfe);
    int c = (ce - cfe);
    int d = (num - ce - cf + cfe);
    int n = a + b + c + d;

    double cp = exp(lgamma(1+a+c) + lgamma(1+b+d) + lgamma(1+a+b)
        + lgamma(1+c+d) - lgamma(1+n) - lgamma(1+a) - lgamma(1+b)
        - lgamma(1+c) - lgamma(1+d));
    double total_p = 0.0;
    int tc = std::min(b,c);
    for (int i = 0; i <= tc; i++)
    {
        total_p += cp;
        double coef = (double)(b)*(double)(c) / (double)(a+1) / (double)(d+1);
        cp *= coef;
        ++a; --c; ++d; --b;
    }
    return total_p;
}
