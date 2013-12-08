cmake_minimum_required (VERSION 2.6)

project(AsyncCache)

include_directories("d:/utils/boost_1_52_0")
add_library(AsyncCache AsyncCache.cpp AsyncCache.h)

install (TARGETS AsyncCache DESTINATION ../bin)