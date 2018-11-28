cmake_minimum_required(VERSION 3.12)
project(reqlang)

set(CMAKE_CXX_STANDARD 17)

add_executable(reqlang main.cpp Token.cpp Token.h Tokenizer.cpp Tokenizer.h)