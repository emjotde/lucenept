#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "LucenePT.h"

int main(int argc, char** argv)
{

    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("lucene,l", po::value<std::string>(), "Path to index")
        ("memory,m", "Load lucene index into memory")
        ("invert,i", "Invert languages and alignment")
        ("phrase-table,p", "Treat input as sentence and output unique subphrases")
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
        std::cerr << "No index given" << std::endl;
        exit(1);
    }

    LucenePT lucenePt(indexPath, vm.count("memory") > 0);
    size_t lines = 0;
    std::string line;
    while (std::getline(std::cin, line))
    {
        if(vm.count("phrase-table") > 0)
            lucenePt.AllPhrases(line, vm.count("invert") > 0);
        else
            lucenePt.CreatePhrase(line, vm.count("invert") > 0);
        std::cerr << lines++ << std::endl;
    }

    return 0;
}
