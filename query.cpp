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

#define _U(x) Lucene::StringUtils::toUnicode(x)
#define _B(x) Lucene::StringUtils::toUTF8(x)

using namespace Lucene;

typedef std::pair<uint8_t, uint8_t> Span;
typedef std::vector<Span> SpanSet;
typedef std::pair<uint32_t, SpanSet> DocSpanSet;
typedef std::vector<DocSpanSet> Hits;
typedef std::map<String, Hits> HitCache;

template <class InputIterator1, class InputIterator2, class OutputIterator>
  OutputIterator phrase_intersection(InputIterator1 first1, InputIterator1 last1,
                                   InputIterator2 first2, InputIterator2 last2,
                                   OutputIterator result) {
    while (first1!=last1 && first2!=last2) {
        if (*first1<*first2) ++first1;
        else if (*first2<*first1) ++first2;
        else {  
            *result = *first1;
            ++result; ++first1; ++first2;
        }
    }
    return result;
}


int main(int argc, char** argv) {
    
    String index(_U(argv[1]));
    IndexReaderPtr reader = IndexReader::open(FSDirectory::open(index), true);
            
    HitCache cache;

    size_t lines = 0, queries = 0;
    std::string line;
    while (std::getline(std::cin, line))
    {
        std::cerr << lines++ << " " << queries << std::endl;
        std::cout << line << std::endl;
        
        String wline(_U(line.c_str()));
        std::vector<String> terms;
        boost::split(terms, wline, boost::is_any_of(L"\t "));
        
        if(terms.empty())
            break;
                
        for(size_t i = 0; i < terms.size(); i++) {
            for(size_t l = 1; l <= terms.size() && i + l <= terms.size(); l++) {
                
                if(l == 1) {
                    std::cout << "\"" << _B(terms[i]) << "\" : ";
                    
                    if(cache.count(terms[i]) == 0) {
                        TermPositionsPtr tp = reader->termPositions(
                            newLucene<Term>(L"source", terms[i]));
                        while(tp->next()) {
                            size_t j = 0;
                            SpanSet spans;
                            while(j++ < tp->freq()) {
                                uint32_t pos = tp->nextPosition();
                                spans.push_back(std::make_pair(pos, pos));
                            }
                            cache[terms[i]].push_back(
                                std::make_pair(tp->doc(), spans));
                        }
                    }
                        
                    queries++;
                    
                    size_t num = cache[terms[i]].size();
                    std::cout << num << std::endl;
                    if(!num)
                        break;
                    
                }
                else {
                    String prefix;
                    
                    Collection<String> phraseTerms = Collection<String>::newInstance();
                    std::wstringstream ss;
                    
                    for(size_t j = 0; j < l; j++) {
                        if(j != 0)
                            ss << _U(" ");
                        
                        ss << terms[i + j];
                        phraseTerms.add(terms[i + j]);
                        
                        if(j == l - 2)
                            prefix = String(ss.str().c_str());
                    }
                    
                    String phrase(ss.str().c_str());
                    
                    std::cout << "\"" << _B(phrase) << "\" : ";
                    
                    String last = terms[i + l - 1];
                    if(cache.count(terms[i]) == 0) {
                        TermPositionsPtr tp = reader->termPositions(
                            newLucene<Term>(L"source", last));
                        while(tp->next()) {
                            size_t j = 0;
                            SpanSet spans;
                            while(j++ < tp->freq()) {
                                uint32_t pos = tp->nextPosition();
                                spans.push_back(std::make_pair(pos, pos));
                            }
                            cache[last].push_back(
                                std::make_pair(tp->doc(), spans));
                        }
                    }
                    
                    Hits hits;
                    phrase_intersection(cache[last].begin(),
                                          cache[last].end(),
                                          cache[prefix].begin(),
                                          cache[prefix].end(),
                                          std::back_inserter(hits));
                    
                    size_t num = hits.size();
                    
                    //std::vector<int> adjacent;
                    //BOOST_FOREACH(int docId, intersection) {
                    //    if(docCache.count(docId) == 0)
                    //        docCache[docId]
                    //            = boost::static_pointer_cast<TermPositionVector>(
                    //                reader->getTermFreqVector(docId, L"source"));
                    //
                    //    TermPositionVectorPtr tpv = docCache[docId];
                    //    
                    //    Collection<int32_t> termIndexes
                    //        = tpv->indexesOf(phraseTerms, 0, l);
                    //    
                    //    Collection<int32_t> starts
                    //            = tpv->getTermPositions(termIndexes[0]);
                    //    
                    //    bool adj = true;
                    //    //BOOST_FOREACH(uint32_t start, starts) {
                    //    //    size_t j = 1;
                    //    //    while(j < l && tpv->getTermPositions(termIndexes[j]).contains(start + j))
                    //    //        j++;
                    //    //    if(j == l)
                    //    //        adj = true;
                    //    //}
                    //    
                    //    if(adj)
                    //        adjacent.push_back(docId);
                    //}
                    
                    
                    std::cout << num << std::endl;
                    if(!num)
                        break;
                    
                    cache[phrase] = hits;
                    
                    //BOOST_FOREACH(int docId, adjacent) {
                    //    DocumentPtr doc = reader->document(docId);
                    //    SingleString source(_B(doc->get(L"source")));
                    //    SingleString target(_B(doc->get(L"target")));
                    //    ByteArray align = doc->getBinaryValue(L"alignment");
                    //    
                    //    std::cout << docId << std::endl;
                    //    std::cout << source << std::endl;
                    //    std::cout << target << std::endl;
                    //    size_t i = 0;
                    //    while(i < align.size()) {
                    //        std::cout << (size_t)align[i++] << "-";
                    //        std::cout << (size_t)align[i++] << " ";
                    //    }
                    //    std::cout << std::endl;
                    //}
                }
            }
        }
        std::cout << std::endl;
    }
    reader->close();

    return 0;
}
