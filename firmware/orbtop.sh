#!/bin/sh
export OBJDUMP=arm-none-eabi-objdump
orbtop --server localhost:6020 --elf-file debug-build/bin/main --routines 20
