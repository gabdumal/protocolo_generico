# Project
add_library(pretty_console pretty_console.cpp)

# Include self
target_include_directories(pretty_console
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
