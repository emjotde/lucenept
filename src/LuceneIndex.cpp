#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iterator>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/FileUtils.h>
#include <lucene++/TermPositions.h>
#include <lucene++/TermPositionVector.h>
#include <lucene++/TermFreqVector.h>

#include "LuceneIndex.h"

#define _U(x) Lucene::StringUtils::toUnicode(x)
#define _U2(x, y) Lucene::StringUtils::toUnicode(x, y)
#define _B(x) Lucene::StringUtils::toUTF8(x)

using namespace Lucene;

LuceneIndex::LuceneIndex(const std::string& directory, bool intoMemory)
    : m_cache(new HitCache()), m_cacheInverse(new HitCache)
{
    DirectoryPtr dir;
    if (intoMemory)
        dir = newLucene<RAMDirectory>(
            FSDirectory::open(_U(directory.c_str())));
    else
        dir = FSDirectory::open(_U(directory.c_str()));

    m_reader = IndexReader::open(dir, true);
}

LuceneIndex::~LuceneIndex()
{
    m_reader->close();
}

size_t LuceneIndex::Size() {
    return m_reader->numDocs();
}

HitsPtr LuceneIndex::GetHits(const std::string& phrase, bool inverse)
{
    String wphrase(_U(phrase.c_str()));
    std::vector<String> terms;
    boost::split(terms, wphrase, boost::is_any_of(L"\t "));
    return GetHits(terms, 0, terms.size(), inverse);
}

HitsPtr LuceneIndex::GetHits(const PhrasePtr& phrase, bool inverse)
{
    const std::vector<StringPiece> tokens = phrase->GetTokens();
    return GetHits(tokens, inverse);
}

HitsPtr LuceneIndex::GetHits(const std::vector<StringPiece>& phraseTerms,
                             bool inverse)
{
    std::vector<String> terms;
    BOOST_FOREACH(StringPiece term, phraseTerms)
        terms.push_back(_U2((const uint8_t*)term.data(), term.length()));
    return GetHits(terms, 0, terms.size(), inverse);
}

HitsPtr LuceneIndex::GetHits(const std::vector<String>& phraseTerms,
                             size_t start,
                             size_t length,
                             bool inverse)
{
    String phrase = PopulateCache(phraseTerms, start, length, inverse);
    if(inverse)
        return (*m_cacheInverse)[phrase];
    return (*m_cache)[phrase];
}

SentencePtr LuceneIndex::GetAlignedSentence(const Hit& hit, bool inverse)
{
    DocumentPtr doc = m_reader->document(hit.doc);
    ByteArray align = doc->getBinaryValue(L"alignment");

    std::string target;
    if(inverse)
        target = _B(doc->get(L"source")).c_str();
    else
        target = _B(doc->get(L"target")).c_str();

    DirectedAlignment alignment;
    int i = 0;
    while (i < align.size())
    {
        uint8_t a = align[i++];
        uint8_t b = align[i++];
        if(inverse) {
            if(alignment.size() <= b)
                alignment.resize(b + 1);
            alignment[b].push_back(a);
        }
        else {
            if(alignment.size() <= a)
                alignment.resize(a + 1);
            alignment[a].push_back(b);
        }
    }
    SentencePtr as(new AlignedTargetSentence(target, alignment));
    return as->shared_from_this();
}

String LuceneIndex::PopulateCache(const std::vector<String>& phraseTerms,
                                  size_t start,
                                  size_t length,
                                  bool inverse)
{
    String penultimo;
    for (size_t j = 0; j < length - 1; j++)
    {
        if (j != 0)
            penultimo.append(_U(" "));
        penultimo.append(phraseTerms[start + j]);
    }
    String ultimo(phraseTerms[start + length - 1]);

    String phrase = ultimo;
    if (length > 1)
        phrase = penultimo + _U(" ") + ultimo;

    HitCachePtr cache = m_cache;
    String field = L"source";
    if(inverse) {
        cache = m_cacheInverse;
        field = L"target";
    }
    if (cache->count(phrase) == 0)
    {
        if (cache->count(ultimo) == 0)
        {
            (*cache)[ultimo] = HitsPtr(new Hits());
            TermPositionsPtr tp = m_reader->termPositions(
                                      newLucene<Term>(field, ultimo));
            while (tp->next())
            {
                int j = 0;
                while (j++ < tp->freq())
                    (*cache)[ultimo]->push_back(
                        Hit(tp->doc(), tp->nextPosition(), 1));
            }
        }
        if (length > 1)
        {
            if (cache->count(penultimo) == 0)
                PopulateCache(phraseTerms, start, length - 1, inverse);

            (*cache)[phrase] = HitsPtr(new Hits());
            PhraseJoinIntersection(
                (*cache)[penultimo]->begin(),
                (*cache)[penultimo]->end(),
                (*cache)[ultimo]->begin(),
                (*cache)[ultimo]->end(),
                std::back_inserter(*(*cache)[phrase]));
        }
    }
    return phrase;
}
