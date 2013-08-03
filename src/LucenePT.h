#ifndef LUCENEPT_HDR
#define LUCENEPT_HDR

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "AlignedSentence.h"

extern double fisher_exact(int, int, int, int);

class LuceneIndex;

class LucenePT
{
public:
    LucenePT(const std::string&, bool = false);

    void AllPhrases(const std::string&, bool = false);
    void CreatePhrase(const std::string&, bool = false);
    void CreatePhrase(const PhrasePtr&, bool = false);

private:
    void CountTargetPhrases(const PhrasePtr&,
                            std::map<PhrasePtr, size_t>&, size_t&,
                            bool = false);

    template <class Iterator>
    HitsPtr TopNSampling(Iterator begin, Iterator end)
    {
        int k = m_maxSamples;
        if(std::distance(begin, end) <= k)
            return HitsPtr(new Hits(begin, end));

        HitsPtr sample(new Hits(begin, begin + k));
        return sample;
    }

    template <class Iterator>
    HitsPtr RandomSampling(Iterator begin, Iterator end)
    {
        // Reservoir Sampling
        int k = m_maxSamples;
        HitsPtr sample = TopNSampling(begin, end);
        for(int i = k; i < std::distance(begin, end); i++)
        {
            boost::random::uniform_int_distribution<> dist(0, i - 1);
            int j = dist(m_gen);
            if(j <= k)
                (*sample)[j] = *(begin + i);
        }
        return sample;
    }

    size_t m_maxSamples;
    size_t m_maxTargetPhrases;
    size_t m_maxPhraseLength;
    boost::shared_ptr<LuceneIndex> m_index;

    // Random number generator
    boost::random::mt19937 m_gen;
};

#endif // LUCENEPT_HDR
