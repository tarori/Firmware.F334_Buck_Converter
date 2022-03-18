openocd.exe -f interface/cmsis-dap.cfg -f target/stm32f3x.cfg -c "program $1; reset; exit"
