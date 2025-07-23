# Versão mínima do CMake e padrões das linguagens
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Bloco de Configuração para a Extensão do VS Code ---
# Essencial para a integração com o VSCode. NÃO REMOVA.
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
# --- Fim do Bloco de Configuração ---

# Define a placa e importa o SDK do Pico
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

# Define o projeto
project(estacao_tu C CXX ASM)

# Inicializa o SDK
pico_sdk_init()

# Adiciona o executável e seus arquivos-fonte
add_executable(${PROJECT_NAME}
    estacao_tu.c
    lib/aht20.c
    lib/bmp280.c
    lib/ssd1306.c
)

# Adiciona os diretórios de include (headers)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lib
)

# Linka as bibliotecas necessárias do SDK
target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    hardware_i2c
    hardware_timer
    hardware_pio
    hardware_pwm
    hardware_adc
    pico_cyw43_arch_lwip_threadsafe_background
)

# Gera o header C a partir do arquivo .pio
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Configurações de saída do programa
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})