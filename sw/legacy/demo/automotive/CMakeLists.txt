# TODO make this library common with the existing one!
add_library(lcd_st7735_lib_am
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/lcd_base.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/lucida_console_10pt.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/lucida_console_12pt.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/core/m3x6_16pt.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers/st7735/lcd_st7735.c
)

add_executable(automotive main.c ksz8851.c lcd.c)
target_link_libraries(automotive common lcd_st7735_lib_am)
target_include_directories(automotive PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/display_drivers)

add_custom_command(
  TARGET automotive POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary "$<TARGET_FILE:automotive>" "$<TARGET_FILE:automotive>.bin"
  COMMAND srec_cat "$<TARGET_FILE:automotive>.bin" -binary -offset 0x0000 -byte-swap 4 -o "$<TARGET_FILE:automotive>.vmem" -vmem
  VERBATIM)
