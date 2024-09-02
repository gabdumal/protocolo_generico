# Sources
target_sources(generic_protocol
    PUBLIC
        protocol.hpp
    PRIVATE
        protocol.cpp
)

# Include self
target_include_directories(generic_protocol
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
