cmake_minimum_required(VERSION 3.25)

project(HelloWorldTree)

set(CMAKE_C_STANDARD 23)

set(LANGUAGE C CXX)

add_executable(HelloWorldTree rbt.h hp.c main.c)
