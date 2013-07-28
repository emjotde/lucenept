#ifndef LUCENEINDEX_HDR
#define LUCENEINDEX_HDR

#include <string>
#include <map>
#include <iterator>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <lucene++/LuceneHeaders.h>

struct Hit
{
    Hit(uint32_t doc, uint8_t start, uint8_t length)
        : doc(doc), start(start), length(length)
    { }

    uint32_t doc;
    uint8_t start;
    uint8_t length;
};

typedef std::pair<uint8_t, uint8_t> AlignPoint;
typedef std::vector<AlignPoint> Alignment;

struct AlignedSentence
{
    AlignedSentence(const std::string& src,
                    const std::string& trg,
                    Alignment align)
        : source(src), target(trg), alignment(align)
    { }

    AlignedSentence(const char* src,
                    const char* trg,
                    Alignment align)
        : source(src), target(trg), alignment(align)
    { }

    std::string source;
    std::string target;
    Alignment alignment;
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

    HitsPtr getHits(const std::string&);
    AlignedSentence hit2AlignedSentence(const Hit&, bool = false);

    size_t printHits(const std::string&);
    void printHitsSentence(const std::string&);

private:
    HitsPtr getHits(std::vector<Lucene::String>&, size_t, size_t);
    size_t printHits(std::vector<Lucene::String>&, size_t, size_t);

    Lucene::String populateCache(std::vector<Lucene::String>&,
                                 size_t, size_t, bool = false);

    template <class InputIterator1,
             class InputIterator2,
             class OutputIterator>
    OutputIterator phrase_join_intersection(InputIterator1 first1,
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

    Lucene::IndexReaderPtr reader_;
    HitCache cache_;
};

#endif
