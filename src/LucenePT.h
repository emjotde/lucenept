#ifndef LUCENEPT_HDR
#define LUCENEPT_HDR

#include <boost/shared_ptr.hpp>
#include "LuceneIndex.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

class LucenePT
{
public:
    LucenePT(const std::string&, bool = false);
    void createPhrase(const std::string&, bool inverse = false);

private:
    void countTargetPhrases(const std::string&,
                            std::map<TargetPhrase, size_t>&, size_t&,
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
    HitsPtr randomSampling(Iterator begin, Iterator end)
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
    boost::random::mt19937 m_gen;
    boost::shared_ptr<LuceneIndex> m_index;
};

#endif // LUCENEPT_HDR
