# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

# Bloco de configuração do VSCode (não mexer)
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(estacao_tu C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(estacao_tu
    estacao_tu.c
    lib/aht20.c
    lib/bmp280.c
    lib/ssd1306.c
)

pico_generate_pio_header(estacao_tu ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_include_directories(estacao_tu PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lib
)

# --- BIBLIOTECAS LIGADAS ---
# Agora só precisamos da biblioteca de rede principal.
# A 'pico_lwip_httpd' foi removida pois não usamos mais o servidor httpd.
target_link_libraries(estacao_tu PRIVATE
    pico_stdlib
    hardware_i2c
    hardware_timer
    hardware_pio
    hardware_pwm
    hardware_adc
    pico_cyw43_arch_lwip_threadsafe_background
)

pico_set_program_name(estacao_tu "estacao_tu")
pico_set_program_version(estacao_tu "0.1")

pico_enable_stdio_usb(estacao_tu 1)
pico_enable_stdio_uart(estacao_tu 0)

pico_add_extra_outputs(estacao_tu)