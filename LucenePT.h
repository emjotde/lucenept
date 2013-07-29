#ifndef LUCENEPT_HDR
#define LUCENEPT_HDR

#include <boost/shared_ptr.hpp>
#include "LuceneIndex.h"

class LucenePT
{
public:
    LucenePT(const std::string&, bool = false);

    void createPhrase(const std::string&);

private:
    boost::shared_ptr<LuceneIndex> index_;
};

#endif // LUCENEPT_HDR
