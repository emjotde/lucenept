cmake_minimum_required(VERSION 2.8.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(LucenePT CXX)
SET(CMAKE_CXX_FLAGS " -Wno-unused-result -Wno-deprecated")
include_directories(${LucenePT_SOURCE_DIR})
set(EXT_LIBS lucene++)

set(LPP_DIR "" CACHE FILEPATH "Path to LucenePlusPlus")
if(NOT EXISTS "${LPP_DIR}/include/lucene++/LuceneHeaders.h")
  message(SEND_ERROR "Can't find LuceneHeaders.h in ${LPP_DIR}/include/lucene++")
endif()
include_directories(${LPP_DIR}/include)
link_directories(${LPP_DIR}/lib)
FIND_LIBRARY(LPP_LIBRARY lucene++ ${LPP_DIR}/lib)
message(STATUS LPP_LIBRARY ${LPP_LIBRARY} )
if(LPP_LIBRARY)
    set(EXT_LIBS ${EXT_LIBS} ${LPP_LIBRARY})
else(LPP_LIBRARY)
    message(SEND_ERROR "Cannot find LPP libraries. Terminating." )
endif(LPP_LIBRARY)

SET(Boost_USE_STATIC_LIBS   ${LUCENE_USE_STATIC_BOOST_LIBS})
SET(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.38.0 COMPONENTS system thread program_options REQUIRED)
IF (Boost_FOUND)
MESSAGE( STATUS "boost found: includes in ${Boost_INCLUDE_DIRS}, library in ${Boost_LIBRARY_DIRS}")
    SET(EXT_LIBS ${EXT_LIBS}
        ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
        ${Boost_SYSTEM_LIBRARY_RELEASE}
        ${Boost_THREAD_LIBRARY_RELEASE})
ENDIF (Boost_FOUND)
                        
set(OBJ_LIBS lucenept)
set(DIRS src)

foreach(dir ${DIRS})
    include_directories(${LucenePT_SOURCE_DIR}/${dir})
endforeach(dir)

foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach(dir)

# ENABLE_TESTING()
# file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/output")
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# ADD_TEST(Options ${CMAKE_CURRENT_BINARY_DIR}/tests/test_options "--log_level=test_suite")
