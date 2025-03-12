add_library(power_manager INTERFACE)

target_sources(power_manager 
    INTERFACE
        power_manager.c
)

target_include_directories(power_manager 
    INTERFACE 
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(power_manager
    INTERFACE
        battery
        utilities
)
