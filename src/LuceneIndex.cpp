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

    m_reader = IndexReader::open(dir, true);
}

LuceneIndex::~LuceneIndex()
{
    m_reader->close();
}

HitsPtr LuceneIndex::GetHits(const std::string& phrase, bool inverse)
{
    String wphrase(_U(phrase.c_str()));
    std::vector<String> terms;
    boost::split(terms, wphrase, boost::is_any_of(L"\t "));
    return GetHits(terms, 0, terms.size(), inverse);
}

HitsPtr LuceneIndex::GetHits(std::vector<String>& phraseTerms,
                             size_t start,
                             size_t length,
                             bool inverse)
{
    String phrase = PopulateCache(phraseTerms, start, length, inverse);
    return m_cache[phrase];
}

AlignedSentencePtr LuceneIndex::GetAlignedSentence(const Hit& hit, bool inverse)
{
    DocumentPtr doc = m_reader->document(hit.doc);
    ByteArray align = doc->getBinaryValue(L"alignment");

    std::string target;
    if(inverse)
    {
        //source = _B(doc->get(L"target")).c_str();
        target = _B(doc->get(L"source")).c_str();
    }
    else
    {
        //source = _B(doc->get(L"source")).c_str();
        target = _B(doc->get(L"target")).c_str();
    }

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
    AlignedSentencePtr as(new AlignedSentence(target, alignment));
    return as->shared_from_this();
}



String LuceneIndex::PopulateCache(std::vector<String>& phraseTerms,
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

    if (m_cache.count(phrase) == 0)
    {
        if (m_cache.count(ultimo) == 0)
        {
            m_cache[ultimo] = HitsPtr(new Hits());
            TermPositionsPtr tp = m_reader->termPositions(
                                      newLucene<Term>(L"source", ultimo));
            while (tp->next())
            {
                int j = 0;
                while (j++ < tp->freq())
                    m_cache[ultimo]->push_back(
                        Hit(tp->doc(), tp->nextPosition(), 1));
            }
        }
        if (length > 1)
        {
            if (m_cache.count(penultimo) == 0)
                PopulateCache(phraseTerms, start, length - 1, inverse);

            m_cache[phrase] = HitsPtr(new Hits());
            PhraseJoinIntersection(
                m_cache[penultimo]->begin(),
                m_cache[penultimo]->end(),
                m_cache[ultimo]->begin(),
                m_cache[ultimo]->end(),
                std::back_inserter(*m_cache[phrase]));
        }
    }
    return phrase;
}

size_t LuceneIndex::PrintHits(std::vector<String>& phraseTerms,
                              size_t start,
                              size_t length)
{
    String phrase = PopulateCache(phraseTerms, start, length);

    std::cout << "\"" << _B(phrase) << "\" : ";
    size_t num = m_cache[phrase]->size();
    std::cout << num << std::endl;

    if (!num)
        return num;

    size_t n = 300;
    HitsPtr hits = m_cache[phrase];
    size_t m = 0;
    BOOST_FOREACH (Hit hit, *hits)
    {
        AlignedSentencePtr as = GetAlignedSentence(hit);

//        std::cout << hit.doc << " [" << (size_t)hit.start
//                  << "," << (size_t)hit.length << "]" << std::endl;

        //std::cout << "Src: " << as.getSource() << std::endl;
        //std::cout << "Trg: " << as.GetTarget() << std::endl;
//        for(size_t i = 0; i < as.GetAlignment().size(); ++i)
//            BOOST_FOREACH(size_t j, as.GetAlignment()[i])
//                std::cout << i << "-" << j << " ";
//        std::cout << std::endl;
        //as.ExtractTargetPhrase(hit.start, hit.length);
        if (m >= n)
            break;
        m++;
    }
    std::cout << std::endl;
    return num;
}

size_t LuceneIndex::PrintHits(const std::string& phrase)
{
    String wphrase(_U(phrase.c_str()));
    std::vector<String> terms;
    boost::split(terms, wphrase, boost::is_any_of(L"\t "));
    return PrintHits(terms, 0, terms.size());
}

void LuceneIndex::PrintHitsSentence(const std::string& sentence)
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
            if (!PrintHits(terms, i, l))
                break;
        }
    }
    std::cout << std::endl;
}
