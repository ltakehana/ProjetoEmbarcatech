# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Definição do projeto
project(Embarcatech_Projeto_Final C CXX ASM)

# Inicializa o Raspberry Pi Pico SDK
pico_sdk_init()

# Adiciona o código-fonte corretamente dentro de src/
add_executable(Embarcatech_Projeto_Final 
    main.c  # Código principal
    src/ssd1306.c 
    src/menu.c
)

# Agora que o target foi criado, podemos adicionar o diretório include
target_include_directories(Embarcatech_Projeto_Final PRIVATE include)

# Define o nome e versão do programa
pico_set_program_name(Embarcatech_Projeto_Final "Embarcatech_Projeto_Final")
pico_set_program_version(Embarcatech_Projeto_Final "0.1")

# Configura saída via USB e desativa UART
pico_enable_stdio_usb(Embarcatech_Projeto_Final 1)
pico_enable_stdio_uart(Embarcatech_Projeto_Final 0)

# Vincula as bibliotecas padrão e as de hardware necessárias
target_link_libraries(Embarcatech_Projeto_Final
    pico_stdlib
    hardware_i2c  # Biblioteca para comunicação com o display OLED
    hardware_adc  # Biblioteca para leitura do ADC
    hardware_pwm  # Biblioteca para controle PWM do MOSFET
    hardware_gpio # Biblioteca para controle de GPIOs
    pico_multicore
)

# Gera arquivos extras para upload
pico_add_extra_outputs(Embarcatech_Projeto_Final)
