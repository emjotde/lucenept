#include <iostream>
#include <string>

#include "LucenePT.h"

int main(int argc, char** argv)
{
    LucenePT lucenePt(argv[1], true);
    size_t lines = 0;
    std::string line;
    while (std::getline(std::cin, line))
    {
        lucenePt.AllPhrases(line, false);
        std::cerr << lines++ << std::endl;
    }

    return 0;
}
