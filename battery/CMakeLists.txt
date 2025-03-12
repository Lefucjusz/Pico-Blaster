add_library(battery INTERFACE)

target_sources(battery
    INTERFACE
        battery.c
)

target_include_directories(battery
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(battery
    INTERFACE
        hardware_adc
        utilities
)
