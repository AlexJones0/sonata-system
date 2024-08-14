add_executable(automotive main.c ksz8851.c)
target_link_libraries(automotive common)

add_custom_command(
  TARGET automotive POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary "$<TARGET_FILE:automotive>" "$<TARGET_FILE:automotive>.bin"
  COMMAND srec_cat "$<TARGET_FILE:automotive>.bin" -binary -offset 0x0000 -byte-swap 4 -o "$<TARGET_FILE:automotive>.vmem" -vmem
  VERBATIM)
