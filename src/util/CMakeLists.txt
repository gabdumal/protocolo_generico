# Project
add_library(util)

# Sources
target_sources(util
    PUBLIC
        util.hpp
    PRIVATE
        util.cpp
)

# Include self
target_include_directories(util
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Linking libraries
target_link_libraries(util
    PRIVATE
        pretty_console
)