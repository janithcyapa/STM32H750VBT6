
st-info --probe

cmake --build build/Debug

openocd -f interface/stlink.cfg -c "reset_config none" -f target/stm32h7x.cfg -c "program build/Debug/STM32H750VBT6.elf verify reset exit"

st-flash --reset write build/Debug/STM32H750VBT6.bin 0x08000000