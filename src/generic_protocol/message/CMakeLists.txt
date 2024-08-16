# Sources
target_sources(generic_protocol
    PUBLIC
        message.hpp
    PRIVATE
        message.cpp
)

# Include self
target_include_directories(generic_protocol
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
