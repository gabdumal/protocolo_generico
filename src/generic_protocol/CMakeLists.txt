# Project
add_library(generic_protocol)

# Sources
target_sources(generic_protocol
    PUBLIC
        generic_protocol.hpp
    PRIVATE
        generic_protocol.cpp
)

# Include self
target_include_directories(generic_protocol
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

# Linking libraries
target_link_libraries(generic_protocol
    PRIVATE
        pretty_console
        uuid
        util
)

# Directories
add_subdirectory(generic_protocol_constants)
add_subdirectory(protocol)
add_subdirectory(message)
add_subdirectory(package)
add_subdirectory(entity)
add_subdirectory(connection)
# add_subdirectory(network)
