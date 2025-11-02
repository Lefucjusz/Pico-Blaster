add_library(utils INTERFACE)

target_include_directories(utils
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(utils
    INTERFACE
        pico_stdlib
)
