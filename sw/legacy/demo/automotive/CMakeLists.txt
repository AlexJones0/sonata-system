add_library(lcd_st7735_lib_am
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/lcd_base.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/lucida_console_10pt.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/lucida_console_12pt.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/m3x6_16pt.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/st7735/lcd_st7735.c
)

add_library(automotive_lib
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/automotive_demo/automotive_common.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/automotive_demo/automotive_menu.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/automotive_demo/no_pedal.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/automotive_demo/joystick_pedal.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/automotive_demo/digital_pedal.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/automotive_demo/analogue_pedal.c
)

add_executable(automotive_send send.c ksz8851.c lcd.c)
target_link_libraries(automotive_send automotive_lib common lcd_st7735_lib_am)
target_include_directories(automotive_send 
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/automotive_demo
)

add_custom_command(
  TARGET automotive_send POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary "$<TARGET_FILE:automotive_send>" "$<TARGET_FILE:automotive_send>.bin"
  COMMAND srec_cat "$<TARGET_FILE:automotive_send>.bin" -binary -offset 0x0000 -byte-swap 4 -o "$<TARGET_FILE:automotive_send>.vmem" -vmem
  VERBATIM)
