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
    void createPhrase(const std::string&, size_t maxTargets = 20);

private:
    template <class Iterator>
    HitsPtr TopNSampling(Iterator begin, Iterator end, size_t k = 300)
    {
        if(std::distance(begin, end) <= k)
            return HitsPtr(new Hits(begin, end));

        HitsPtr sample(new Hits(begin, begin + k));
        return sample;
    }

    template <class Iterator>
    HitsPtr randomSampling(Iterator begin, Iterator end, size_t k = 300)
    {
        // Reservoir Sampling
        HitsPtr sample = TopNSampling(begin, end, k);
        for(size_t i = k; i < std::distance(begin, end); i++)
        {
            boost::random::uniform_int_distribution<> dist(0, i - 1);
            size_t j = dist(m_gen);
            if(j <= k)
                (*sample)[j] = *(begin + i);
        }
        return sample;
    }

    size_t m_maxSamples;
    boost::random::mt19937 m_gen;
    boost::shared_ptr<LuceneIndex> index_;
};

#endif // LUCENEPT_HDR
