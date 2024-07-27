# Executables
add_executable(dcc042 main.cpp)

# Directories
add_subdirectory(generic_protocol)

target_link_libraries(dcc042
    PRIVATE
        generic_protocol
)