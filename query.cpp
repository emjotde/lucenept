#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/FileUtils.h>

#define _U(x) Lucene::StringUtils::toUnicode(x)
#define _B(x) Lucene::StringUtils::toUTF8(x)

using namespace Lucene;

int main(int argc, char** argv) {
    
    String index(_U(argv[1]));
    IndexReaderPtr reader = IndexReader::open(FSDirectory::open(index), true);

    SearcherPtr searcher = newLucene<IndexSearcher>(reader);
    AnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
   
    std::map<SingleString, bool> cache;

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
                    
                    if(cache.count(_B(terms[i])) > 0) {
                        std::cout << "Cached" << std::endl;
                        continue;
                    }
                    
                    size_t num = reader->docFreq(newLucene<Term>(L"source",
                                                                 terms[i]));
                    
                    queries++;
                    
                    cache[_B(terms[i])] = true;
                    
                    std::cout << num << std::endl;
                    if(!num)
                        break;
                }
                else {
                    PhraseQueryPtr query = newLucene<PhraseQuery>();
                    for(size_t j = 0; j < l; j++)
                        query->add(newLucene<Term>(L"source", terms[i + j]));
                    
                    std::cout << _B(query->toString(L"source")) << " : ";
                    
                    if(cache.count(_B(query->toString(L"source"))) > 0) {
                        std::cout << "Cached" << std::endl;
                        continue;
                    }
                    
                    TopDocsPtr topDocs = searcher->search(query, 100);
                    queries++;
        
                    size_t num = topDocs->totalHits;
                    cache[_B(query->toString(L"source"))] = true;
                    
                    std::cout << num << std::endl;
                    if(!num)
                        break;
                    
                    //BOOST_FOREACH(ScoreDocPtr scoreDoc, topDocs->scoreDocs) {
                    //    DocumentPtr doc = reader->document(scoreDoc->doc);
                    //    SingleString source(_B(doc->get(L"source")));
                    //    SingleString target(_B(doc->get(L"target")));
                    //    ByteArray align = doc->getBinaryValue(L"alignment");
                    //    
                    //    std::cout << scoreDoc->score << std::endl;
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
