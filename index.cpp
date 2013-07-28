#include <iostream>
#include <fstream>
#include <string>
#include <boost/foreach.hpp>
#include <sstream>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/FileUtils.h>

#define _U(x) Lucene::StringUtils::toUnicode(x)
#define _B(x) Lucene::StringUtils::toUTF8(x)

using namespace Lucene;

int main(int argc, char** argv)
{

    if(argc < 5)
    {
        std::cerr << "Wrong arguments" << std::endl;
        exit(1);
    }

    String indexDir(_U(argv[1]));
    std::cout << _B(indexDir) << std::endl;

    std::ifstream sourceIn(argv[2]), targetIn(argv[3]), alignIn(argv[4]);

    if (!FileUtils::isDirectory(indexDir))
        FileUtils::createDirectory(indexDir);

    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexDir),
                            newLucene<WhitespaceAnalyzer>(),
                            true, IndexWriter::MaxFieldLengthLIMITED);

    SingleString source, target, align;
    size_t lines = 0;
    while(std::getline(sourceIn, source)
            && std::getline(targetIn, target)
            && std::getline(alignIn, align))
    {

        if(lines % 10000 == 0)
            std::cerr << lines << std::endl;

        DocumentPtr doc = newLucene<Document>();

        doc->add(newLucene<Field>(L"source",
                                  _U(source),
                                  Field::STORE_YES,
                                  Field::INDEX_ANALYZED,
                                  Field::TERM_VECTOR_WITH_POSITIONS));

        doc->add(newLucene<Field>(L"target",
                                  _U(target),
                                  Field::STORE_YES,
                                  Field::INDEX_ANALYZED,
                                  Field::TERM_VECTOR_WITH_POSITIONS));

        ByteArray byteArray;
        byteArray.resize(align.size());

        std::stringstream alignStr(align.c_str());
        size_t i = 0, a, b;
        char hyphen;
        while(alignStr >> a >> hyphen >> b)
        {
            byteArray[i++] = (uint8_t)a;
            byteArray[i++] = (uint8_t)b;
        }
        byteArray.resize(i);

        doc->add(newLucene<Field>(L"alignment",
                                  byteArray,
                                  Field::STORE_YES));

        writer->addDocument(doc);
        lines++;
    }

    writer->optimize();
    writer->close();

    return 0;
}
