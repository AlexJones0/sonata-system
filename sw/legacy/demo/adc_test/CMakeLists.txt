add_executable(adc_test main.c)
target_link_libraries(adc_test common)

add_custom_command(
  TARGET adc_test POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary "$<TARGET_FILE:adc_test>" "$<TARGET_FILE:adc_test>.bin"
  COMMAND srec_cat "$<TARGET_FILE:adc_test>.bin" -binary -offset 0x0000 -byte-swap 4 -o "$<TARGET_FILE:adc_test>.vmem" -vmem
  VERBATIM)
