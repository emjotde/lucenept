#include <iostream>
#include <fstream>
#include <string>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <sstream>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/FileUtils.h>

#define _U(x) Lucene::StringUtils::toUnicode(x)
#define _B(x) Lucene::StringUtils::toUTF8(x)

using namespace Lucene;

int main(int argc, char** argv)
{
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("lucene,l", po::value<std::string>(), "Path to index")
        ("source,s", po::value<std::string>(), "Path to source sentences")
        ("target,t", po::value<std::string>(), "Path to target sentences")
        ("align,a", po::value<std::string>(), "Path to alignments")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    std::string indexPath;
    if (vm.count("lucene")) {
        indexPath = vm["lucene"].as<std::string>();
    } else {
        std::cerr << "No lucene index given" << std::endl;
        exit(1);
    }

    std::string sourcePath;
    if (vm.count("source")) {
        sourcePath = vm["source"].as<std::string>();
    } else {
        std::cerr << "No source file given" << std::endl;
        exit(1);
    }

    std::string targetPath;
    if (vm.count("target")) {
        targetPath = vm["target"].as<std::string>();
    } else {
        std::cerr << "No target file given" << std::endl;
        exit(1);
    }

    std::string alignPath;
    if (vm.count("align")) {
        alignPath = vm["align"].as<std::string>();
    } else {
        std::cerr << "No align file given" << std::endl;
        exit(1);
    }

    std::ifstream sourceIn(sourcePath.c_str()), targetIn(targetPath.c_str()), alignIn(alignPath.c_str());

    String indexDir(_U(indexPath.c_str()));
    std::cout << _B(indexDir) << std::endl;
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
