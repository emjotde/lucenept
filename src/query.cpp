#include <iostream>
#include <string>

#include "LucenePT.h"

int main(int argc, char** argv)
{
    LucenePT lucenePt(argv[1], true);
    std::string line;
    while (std::getline(std::cin, line))
    {
        lucenePt.createPhrase(line, false);
        //std::cout << "--" << std::endl;
        //lucenePt.createPhrase(line, true);
    }

    return 0;
}
