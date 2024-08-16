# Project
add_library(constants INTERFACE)

# Include self
target_include_directories(constants
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
