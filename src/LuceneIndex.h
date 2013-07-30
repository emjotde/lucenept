#ifndef LUCENEINDEX_HDR
#define LUCENEINDEX_HDR

#include <string>
#include <map>
#include <iterator>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <lucene++/LuceneHeaders.h>

#include "AlignedSentence.h"

struct Hit
{
    Hit(uint32_t doc, uint8_t start, uint8_t length)
        : doc(doc), start(start), length(length)
    { }

    uint32_t doc;
    uint8_t start;
    uint8_t length;
};

typedef std::vector<Hit> Hits;
typedef boost::shared_ptr<Hits> HitsPtr;

class LuceneIndex
{
private:
    typedef boost::unordered_map<Lucene::String, HitsPtr> HitCache;

public:
    LuceneIndex(const std::string&, bool = false);
    ~LuceneIndex();

    HitsPtr GetHits(const std::string&, bool = false);
    AlignedSentencePtr GetAlignedSentence(const Hit&, bool = false);

    size_t PrintHits(const std::string&);
    void PrintHitsSentence(const std::string&);

private:
    HitsPtr GetHits(std::vector<Lucene::String>&, size_t, size_t, bool = false);
    size_t PrintHits(std::vector<Lucene::String>&, size_t, size_t);

    Lucene::String PopulateCache(std::vector<Lucene::String>&,
                                 size_t, size_t, bool = false);

    template <class InputIterator1,
             class InputIterator2,
             class OutputIterator>
    OutputIterator PhraseJoinIntersection(InputIterator1 first1,
                                            InputIterator1 last1,
                                            InputIterator2 first2,
                                            InputIterator2 last2,
                                            OutputIterator result)
    {
        while (first1 != last1 && first2 != last2)
        {
            if (first1->doc < first2->doc)
                ++first1;
            else if (first2->doc < first1->doc)
                ++first2;
            else
            {
                if (first1->start + first1->length < first2->start)
                    ++first1;
                else if (first2->start < first1->start + first1->length)
                    ++first2;
                else
                {
                    *result = Hit(first1->doc,
                                  first1->start,
                                  first1->length + first2->length);
                    ++result;
                    ++first1;
                    ++first2;
                }
            }
        }
        return result;
    }

    Lucene::IndexReaderPtr m_reader;
    HitCache m_cache;
};

#endif
