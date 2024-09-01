# Executables
add_executable(networks_project
    main.cpp
)

# Include directories
target_include_directories(networks_project
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

# Linking libraries
target_link_libraries(networks_project
    PRIVATE
        pretty_console
        uuid
        util
        generic_protocol
)

# Directories
add_subdirectory(util)
add_subdirectory(generic_protocol)
