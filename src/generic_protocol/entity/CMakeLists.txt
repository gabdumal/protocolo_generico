# Sources
target_sources(generic_protocol
    PUBLIC
        entity.hpp
    PRIVATE
        entity.cpp
)

# Include self
target_include_directories(generic_protocol
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)