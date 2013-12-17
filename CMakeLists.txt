cmake_minimum_required(VERSION 2.8.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(LucenePT CXX)
SET(CMAKE_CXX_FLAGS " -Wno-unused-result -Wno-deprecated")
include_directories(${LucenePT_SOURCE_DIR})

set(EXT_LIBS)

find_package(LucenePlusPlus REQUIRED)
if(LucenePlusPlus_FOUND)
    include_directories(${LucenePlusPlus_INCLUDE_DIRS})
    SET(EXT_LIBS ${EXT_LIBS}
        ${LucenePlusPlus_LIBRARIES})
endif(LucenePlusPlus_FOUND)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.38.0 COMPONENTS program_options system REQUIRED)
IF (Boost_FOUND)
MESSAGE( STATUS "boost found: includes in ${Boost_INCLUDE_DIRS}, library in ${Boost_LIBRARY_DIRS}")
    SET(EXT_LIBS ${EXT_LIBS}
        ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
        ${Boost_SYSTEM_LIBRARY_RELEASE})
ENDIF (Boost_FOUND)
                        
set(DIRS src)

foreach(dir ${DIRS})
    include_directories(${LucenePT_SOURCE_DIR}/${dir})
endforeach(dir)

foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach(dir)
