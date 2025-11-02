add_library(battery STATIC)

target_sources(battery
    PRIVATE
        battery.c
)

target_include_directories(battery
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(battery
    PRIVATE
        hardware_adc
        utilities
)
