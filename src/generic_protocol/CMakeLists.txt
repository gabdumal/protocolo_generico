# Project
add_library(generic_protocol)

# Sources
target_sources(generic_protocol
    PUBLIC
        generic_protocol.hpp
    PRIVATE
        generic_protocol.cpp
)

# Directories
target_include_directories(generic_protocol
     PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)