add_library(power_manager STATIC)

target_sources(power_manager 
    PRIVATE
        power_manager.c
)

target_include_directories(power_manager 
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(power_manager
    PRIVATE
        battery
        utilities
)
