#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iterator>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

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
typedef boost::unordered_map<String, Hits> HitCache;

template <class InputIterator1, class InputIterator2, class OutputIterator>
  OutputIterator phrase_join_intersection(InputIterator1 first1, InputIterator1 last1,
                                   InputIterator2 first2, InputIterator2 last2,
                                   OutputIterator result) {
    while (first1 != last1 && first2 != last2) {
        if (first1->first < first2->first)
            ++first1;
        else if (first2->first < first1->first)
            ++first2;
        else {
            SpanSet& prefix = first1->second;
            SpanSet& last   = first2->second;
            
            SpanSet adjacent;
            BOOST_FOREACH(Span s1, prefix) {
                BOOST_FOREACH(Span s2, last) {
                    if(s1.second + 1 == s2.first) {
                        adjacent.push_back(std::make_pair(s1.first, s2.second));
                        continue;
                    }
                }
            }
            
            if(!adjacent.empty()) {
                *result = DocSpanSet(first1->first, adjacent);
                ++result;
            }
            
            ++first1; ++first2;
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
        std::cerr << lines++ << std::endl;
        std::cout << line << std::endl;
        
        String wline(_U(line.c_str()));
        std::vector<String> terms;
        boost::split(terms, wline, boost::is_any_of(L"\t "));
        
        if(terms.empty())
            break;
                
        for(size_t i = 0; i < terms.size(); i++) {
            for(size_t l = 1; l <= terms.size() && i + l <= terms.size(); l++) {
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
                
                if(cache.count(phrase) == 0) {
                    String last = terms[i + l - 1];
                    if(cache.count(last) == 0) {
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
                    
                    if(l > 1) {
                        phrase_join_intersection(cache[prefix].begin(),
                            cache[prefix].end(),
                            cache[last].begin(),
                            cache[last].end(),
                            std::back_inserter(cache[phrase]));
                    }    
                }
                
                std::cout << "\"" << _B(phrase) << "\" : ";
                size_t num = cache[phrase].size();
                std::cout << num << std::endl;  
                
                if(!num)
                    break;
                
                Hits nhits;
                size_t n = 10;
                
                Hits& hits = cache[phrase];
                //std::random_shuffle(hits.begin(), hits.end());
                std::copy(hits.begin(), hits.begin() + (n < hits.size() ? n
                                                        : hits.size()),
                          std::back_inserter(nhits));
                
                BOOST_FOREACH(DocSpanSet docSpanSet, nhits) {
                    int docId = docSpanSet.first;
                    DocumentPtr doc = reader->document(docId);
                    SingleString source(_B(doc->get(L"source")));
                    SingleString target(_B(doc->get(L"target")));
                    ByteArray align = doc->getBinaryValue(L"alignment");
                    
                    BOOST_FOREACH(Span s, docSpanSet.second)
                        std::cout << docId << " [" << (size_t)s.first << ","
                        << (size_t)s.second << "]" << std::endl;
                    std::cout << source << std::endl;
                    std::cout << target << std::endl;
                    size_t i = 0;
                    while(i < align.size()) {
                        std::cout << (size_t)align[i++] << "-";
                        std::cout << (size_t)align[i++] << " ";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
    reader->close();

    return 0;
}
