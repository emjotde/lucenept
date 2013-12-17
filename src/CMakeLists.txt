
add_library(lucenept STATIC
    AlignedSentence.cpp
    LuceneIndex.cpp
    LucenePT.cpp
)

add_executable(index index.cpp)
target_link_libraries(index ${EXT_LIBS})

add_executable(query query.cpp)
target_link_libraries(query lucenept ${EXT_LIBS})

