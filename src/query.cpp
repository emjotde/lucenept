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
        ("index,i", po::value<std::string>(), "Path to index")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    std::string indexPath;
    if (vm.count("index")) {
        indexPath = vm["index"].as<std::string>();
    } else {
        std::cerr << "No index given" << std::endl;
        exit(1);
    }

    LucenePT lucenePt(indexPath, true);
    size_t lines = 0;
    std::string line;
    while (std::getline(std::cin, line))
    {
        lucenePt.AllPhrases(line, false);
        std::cerr << lines++ << std::endl;
    }

    return 0;
}
