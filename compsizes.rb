#!/usr/bin/env ruby

GCC = 'arm-none-eabi-gcc'

`#{GCC} -mthumb -mcpu=cortex-m3 -DPART_lpc1343 -ffunction-sections -fdata-sections -MD -std=gnu99 -Wall -Os -c uformat.c -o uformat.c.o`
`arm-none-eabi-objcopy -O binary uformat.c.o uformat_large.bin`
`#{GCC} -mthumb -mcpu=cortex-m3 -DPART_lpc1343 -DUPRINTF_TINY -ffunction-sections -fdata-sections -MD -std=gnu99 -Wall -Os -c uformat.c -o uformat.c.o`
`arm-none-eabi-objcopy -O binary uformat.c.o uformat_tiny.bin`
puts "Full: #{File.size('uformat_large.bin')} B"
puts "Tiny: #{File.size('uformat_tiny.bin')} B"