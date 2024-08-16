# Sources
target_sources(generic_protocol
    PUBLIC
        network.hpp
    PRIVATE
        network.cpp
)

# Include self
target_include_directories(generic_protocol
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
