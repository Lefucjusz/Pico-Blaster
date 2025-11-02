add_library(bluetooth STATIC)

target_sources(bluetooth
    PRIVATE
        a2dp.c
        avrcp.c
        bt.c
        sdp.c
        bt_i2s.c
)

target_include_directories(bluetooth
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(bluetooth
    PUBLIC
        pico_cyw43_arch_none
        pico_btstack_cyw43
        pico_btstack_classic
        pico_btstack_sbc_decoder
    PRIVATE
        audio_i2s
        dsp
)
