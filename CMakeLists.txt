cmake_minimum_required(VERSION 3.15)

project(HIB_Parser)

set(RUN_TEST OFF)

if(RUN_TEST)
  add_executable(test_parser
    main.cpp
    parser.h
    parser.cpp
  )
else()
  add_library(HIB_Parser
    parser.h
    parser.cpp
  )
endif()
