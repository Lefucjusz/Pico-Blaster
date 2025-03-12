add_library(utilities INTERFACE)

add_subdirectory(filter)
add_subdirectory(utils)

target_link_libraries(utilities
    INTERFACE
        filter
        utils
)
