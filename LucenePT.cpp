#include "LucenePT.h"
#include <boost/foreach.hpp>

LucenePT::LucenePT(const std::string& dir, bool intoMemory)
    : index_(new LuceneIndex(dir, intoMemory))
{ }

void LucenePT::createPhrase(const std::string& phrase)
{
    HitsPtr hits = index_->getHits(phrase);
    BOOST_FOREACH(Hit hit, *hits)
    {
        size_t start = hit.start;
        size_t length = hit.length;
        AlignedSentence as = index_->getAlignedSentence(hit);
    }
}
