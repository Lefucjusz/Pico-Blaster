add_library(dsp STATIC)

target_sources(dsp
    PRIVATE
        biquad.c
        dsp.c
)

target_include_directories(dsp
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(dsp
    PRIVATE
        utilities
)
