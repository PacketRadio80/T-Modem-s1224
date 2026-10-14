# T-Modem-s1224 Pico firmware — softmodem · three UF2 images (build separately)
cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
  message(FATAL_ERROR "Set PICO_SDK_PATH to the Raspberry Pi Pico SDK root")
endif()
if(DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
  set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(s1224_firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

if(NOT DEFINED S1224_BITRATE)
  set(S1224_BITRATE 1200)
endif()

if(S1224_BITRATE EQUAL 300)
  set(S1224_TARGET s1224-300)
elseif(S1224_BITRATE EQUAL 600)
  set(S1224_TARGET s1224-600)
elseif(S1224_BITRATE EQUAL 1200)
  set(S1224_TARGET s1224-1200)
elseif(S1224_BITRATE EQUAL 2400)
  set(S1224_TARGET s1224-2400)
elseif(S1224_BITRATE EQUAL 3600)
  set(S1224_TARGET s1224-3600)
elseif(S1224_BITRATE EQUAL 4800)
  set(S1224_TARGET s1224-4800)
else()
  message(FATAL_ERROR "S1224_BITRATE must be 300, 600, 1200, 2400, 3600, or 4800")
endif()

add_executable(${S1224_TARGET}
  src/common/main.c
  src/common/status.c
  src/common/ptt.c
  src/common/modem_probe.c
  src/common/usb_cmd.c
  src/common/display.c
  src/common/board_pins.c
  src/common/crystal_steer.c
  src/common/baud_config.c
  src/common/config.c
  src/common/sense.c
  src/common/hdlc.c
  src/common/kiss.c
  src/common/afsk_soft.c
  src/common/banner.c
)

target_include_directories(${S1224_TARGET} PRIVATE include)
target_compile_definitions(${S1224_TARGET} PRIVATE S1224_BITRATE=${S1224_BITRATE})
target_link_libraries(${S1224_TARGET}
  pico_stdlib
  hardware_gpio
  hardware_i2c
  hardware_flash
  hardware_sync
  hardware_pwm
  hardware_adc
  m
)

pico_enable_stdio_usb(${S1224_TARGET} 1)
pico_enable_stdio_uart(${S1224_TARGET} 0)
pico_add_extra_outputs(${S1224_TARGET})
pico_set_binary_type(${S1224_TARGET} default)
