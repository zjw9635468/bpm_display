cmake_minimum_required(VERSION 3.8)

project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        SOURCE_DIR "/Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/googletest-src"
        BINARY_DIR "/Users/mac/Desktop/Learning/CS/cs174b/cs174b_pa01/build/googletest-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        )
