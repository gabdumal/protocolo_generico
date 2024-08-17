# Project
target_sources(generic_protocol
    PUBLIC
        generic_protocol_constants.hpp
)

# Include self
target_include_directories(generic_protocol
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
