# Hardcode os path into OS_DIR for excluding files and directories by the pass

set(SOURCE_PATH ${CMAKE_SOURCE_DIR})
cmake_path(GET SOURCE_PATH PARENT_PATH OS_PATH)

add_library(Mpass MODULE
    # List your source files here.
    passrunner.cpp
)
set(OS_PATH_PROCESSED "\"${OS_PATH}\"")
target_compile_definitions(Mpass PRIVATE OS_DIR=${OS_PATH_PROCESSED})

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(Mpass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(Mpass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(Mpass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
