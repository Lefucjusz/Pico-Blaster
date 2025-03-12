add_library(filter STATIC)

target_sources(filter
    PRIVATE
        filter.c
)

target_include_directories(filter
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
