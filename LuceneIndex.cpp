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
#define _B(x) Lucene::StringUtils::toUTF8(x)

using namespace Lucene;

LuceneIndex::LuceneIndex(const std::string& directory, bool intoMemory)
{
    DirectoryPtr dir;
    if (intoMemory)
        dir = newLucene<RAMDirectory>(
                  FSDirectory::open(_U(directory.c_str())));
    else
        dir = FSDirectory::open(_U(directory.c_str()));

    reader_ = IndexReader::open(dir, true);
}

LuceneIndex::~LuceneIndex()
{
    reader_->close();
}

HitsPtr LuceneIndex::getHits(const std::string& phrase, bool inverse)
{
    String wphrase(_U(phrase.c_str()));
    std::vector<String> terms;
    boost::split(terms, wphrase, boost::is_any_of(L"\t "));
    return getHits(terms, 0, terms.size(), inverse);
}

HitsPtr LuceneIndex::getHits(std::vector<String>& phraseTerms,
                             size_t start,
                             size_t length,
                             bool inverse)
{
    String phrase = populateCache(phraseTerms, start, length, inverse);
    return cache_[phrase];
}

AlignedSentence LuceneIndex::hit2AlignedSentence(const Hit& hit, bool inverse)
{
    DocumentPtr doc = reader_->document(hit.doc);
    ByteArray align = doc->getBinaryValue(L"alignment");

    std::string source, target;
    if(inverse)
    {
        source = _B(doc->get(L"target")).c_str();
        target = _B(doc->get(L"source")).c_str();
    }
    else
    {
        source = _B(doc->get(L"source")).c_str();
        target = _B(doc->get(L"target")).c_str();
    }

    AlignedSentence as(source, target, Alignment());

    int i = 0;
    while (i < align.size())
    {
        uint8_t a = align[i++];
        uint8_t b = align[i++];
        if(inverse)
            as.alignment.push_back(AlignPoint(b, a));
        else
            as.alignment.push_back(AlignPoint(a, b));
    }

    return as;
}

String LuceneIndex::populateCache(std::vector<String>& phraseTerms,
                                  size_t start,
                                  size_t length,
                                  bool inverse)
// TODO (marcinj#6#): Add implementation for inverse = true
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

    if (cache_.count(phrase) == 0)
    {
        if (cache_.count(ultimo) == 0)
        {
            cache_[ultimo] = HitsPtr(new Hits());
            TermPositionsPtr tp = reader_->termPositions(
                                      newLucene<Term>(L"source", ultimo));
            while (tp->next())
            {
                int j = 0;
                while (j++ < tp->freq())
                    cache_[ultimo]->push_back(
                        Hit(tp->doc(), tp->nextPosition(), 1));
            }
        }
        if (length > 1)
        {
            if (cache_.count(penultimo) == 0)
                populateCache(phraseTerms, start, length - 1, inverse);

            cache_[phrase] = HitsPtr(new Hits());
            phrase_join_intersection(
                cache_[penultimo]->begin(),
                cache_[penultimo]->end(),
                cache_[ultimo]->begin(),
                cache_[ultimo]->end(),
                std::back_inserter(*cache_[phrase]));
        }
    }
    return phrase;
}

size_t LuceneIndex::printHits(std::vector<String>& phraseTerms,
                              size_t start,
                              size_t length)
{
    String phrase = populateCache(phraseTerms, start, length);

    std::cout << "\"" << _B(phrase) << "\" : ";
    size_t num = cache_[phrase]->size();
    std::cout << num << std::endl;

    if (!num)
        return num;

    size_t n = 10;
    HitsPtr hits = cache_[phrase];
    size_t m = 0;
    BOOST_FOREACH (Hit hit, *hits)
    {
        AlignedSentence as = hit2AlignedSentence(hit);

        std::cout << hit.doc << " [" << (size_t)hit.start
                  << "," << (size_t)hit.length << "]" << std::endl;

        std::cout << "Src: " << as.source << std::endl;
        std::cout << "Trg: " << as.target << std::endl;
        BOOST_FOREACH(AlignPoint a, as.alignment)
        std::cout << (size_t)a.first << "-" << (size_t)a.second << " ";
        std::cout << std::endl;
        if (m >= n)
            break;
        m++;
    }
    std::cout << std::endl;
    return num;
}

size_t LuceneIndex::printHits(const std::string& phrase)
{
    String wphrase(_U(phrase.c_str()));
    std::vector<String> terms;
    boost::split(terms, wphrase, boost::is_any_of(L"\t "));
    return printHits(terms, 0, terms.size());
}

void LuceneIndex::printHitsSentence(const std::string& sentence)
{
    String wsentence(_U(sentence.c_str()));
    std::vector<String> terms;
    boost::split(terms, wsentence, boost::is_any_of(L"\t "));

    if(terms.empty())
        return;

    for (size_t i = 0; i < terms.size(); i++)
    {
        for (size_t l = 1; l <= terms.size() && i + l <= terms.size(); l++)
        {
            if (!printHits(terms, i, l))
                break;
        }
    }
    std::cout << std::endl;
}
