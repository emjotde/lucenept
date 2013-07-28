#include <iostream>
#include <string>

#include "LuceneIndex.h"

int main(int argc, char** argv)
{

    LuceneIndex luceneIndex(argv[1], true);

    size_t lines = 0;
    std::string line;
    while (std::getline(std::cin, line))
    {
        std::cerr << lines++ << std::endl;
        std::cout << line << std::endl;

        luceneIndex.printHitsSentence(line);
    }

    return 0;
}
