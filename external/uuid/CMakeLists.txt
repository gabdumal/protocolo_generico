# Project
add_library(uuid INTERFACE)

# Include self
target_include_directories(uuid
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
