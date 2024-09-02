# Sources
target_sources(generic_protocol
    PUBLIC
        connection.hpp
    PRIVATE
        connection.cpp
)

# Include self
target_include_directories(generic_protocol
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)